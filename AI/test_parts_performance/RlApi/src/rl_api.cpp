#include <rl_api.h>

using namespace std;
using namespace everest;
using namespace everest::planner;

uint64_t RlApi::time_tToTimestamp(const time_t &t ){
    return (((uint64_t)t) * (uint64_t)10000000) + ((uint64_t)116444736*1000000000);
}

uint64_t RlApi::get_sys_time_interval(){
    timespec  tim;
    clock_gettime(CLOCK_MONOTONIC, &tim);
    return (time_tToTimestamp( tim.tv_sec ) + tim.tv_nsec/100)/10000;
}

void RlApi::printRKNNTensor(rknn_tensor_attr *attr) {
    printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n", 
            attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2], attr->dims[1], attr->dims[0], 
            attr->n_elems, attr->size, 0, attr->type, attr->qnt_type, attr->fl, attr->zp, attr->scale);
}

unsigned char* RlApi::load_model(const char *filename, int *model_size)
{
    FILE *fp = fopen(filename, "rb");
    if(fp == nullptr) {
        printf("fopen %s fail!\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    unsigned char *model = (unsigned char*)malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if(model_len != fread(model, 1, model_len, fp)) {
        printf("fread %s fail!\n", filename);
        free(model);
        return NULL;
    }
    *model_size = model_len;
    if(fp) {
        fclose(fp);
    }
    return model;
}

int RlApi::predictions(vector<vector<vector<double>>> inputs_map, int expand_type, vector<vector<double>> mask, int &res_idx, int &res_idy) {
	// const int img_width = 224;
	// const int img_height = 224;
	// const int img_channels = 3;
	printf("========== START GETTING TARGET ==========\n");
	rknn_context ctx;
	int ret;
	int model_len = 0;
	unsigned char* model;

	printf("==== Size of input maps(TOOOO MODEL):::===> inputs_map.size()=%d, inputs_map[0].size()=%d, inputs_map[0][0].size()=%d\n", inputs_map.size(), inputs_map[0].size(), inputs_map[0][0].size());
	printf("==== Values of input maps:::===> inputs_map[0][0][0]=%f, inputs_map[0][1][2]=%f, inputs_map[1][100][234]=%f, inputs_map[5][100][234]=%f\n", inputs_map[0][0][0], inputs_map[0][1][2], inputs_map[1][100][234], inputs_map[5][100][234]);

	const char* model_path = "/userdata/model/ckpt_precompile_20_rk161.rknn";
	printf("mode path is : %s\n", model_path);
	// const char* img_path = argv[2];
	// vector<vector<vector<double>>> data_vector;
	// data_vector = get_inputs();
	// uchar batch_img_data[img.cols*img.rows*img.channels() * BATCH_SIZE];
	uchar batch_img_data[240 * 240 * 8 * BATCH_SIZE];
	uchar data[240 * 240 * 8];
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 240; j++) {
			for (int k = 0; k < 240; k++) {
				data[i * 8 + j * 240 + k] = inputs_map[i][j][k];
			}
		}
	}

	// const char *img_path2 = argv[3];
	// unsigned long start_time,end_load_model_time, stop_time;
	timeval start_time, end_load_model_time, end_init_time, end_run_time, end_process_time, stop_time;
	gettimeofday(&start_time, nullptr);
	// start_time = GetTickCount();
	long startt = get_sys_time_interval();

	// Load image
	// cv::Mat img = cv::imread(img_path);
	// img = get_net_work_img(img);

	// memcpy(batch_img_data, img.data, img.cols*img.rows*img.channels());
	memcpy(batch_img_data, data, 240 * 240 * 8);
	// data -> const char*
	printf("batch_img_data[0]=%d, batch_img_data[53]=%d, batch_img_data[245]=%d, batch_img_data[465]=%d\n", batch_img_data[0], batch_img_data[53], batch_img_data[245], batch_img_data[465]);
	printf("===== load input data done =====\n");

	// Load RKNN Model
	model = load_model(model_path, &model_len);
	gettimeofday(&end_load_model_time, nullptr);
	// end_load_model_time = GetTickCount();
	long end_load_model = get_sys_time_interval();
	printf("end load model time:%ldms\n", end_load_model);
	ret = rknn_init(&ctx, model, model_len, 0);
	gettimeofday(&end_init_time, nullptr);
	// end_load_model_time = GetTickCount();
	long end_init = get_sys_time_interval();
	printf("end init model time:%ldms\n", end_init);
	if (ret < 0) {
		printf("rknn_init fail! ret=%d\n", ret);
		return -1;
	}

	////// Get Model Input Output Info
	rknn_input_output_num io_num;
	ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
	if (ret != RKNN_SUCC) {
		printf("rknn_query fail! ret=%d\n", ret);
		return -1;
	}
	printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

	printf("input tensors:\n");
	rknn_tensor_attr input_attrs[io_num.n_input];
	memset(input_attrs, 0, sizeof(input_attrs));
	for (int i = 0; i < io_num.n_input; i++) {
		input_attrs[i].index = i;
		ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
		if (ret != RKNN_SUCC) {
			printf("rknn_query fail! ret=%d\n", ret);
			return -1;
		}
		printRKNNTensor(&(input_attrs[i]));
	}

	printf("output tensors:\n");
	rknn_tensor_attr output_attrs[io_num.n_output];
	memset(output_attrs, 0, sizeof(output_attrs));
	for (int i = 0; i < io_num.n_output; i++) {
		output_attrs[i].index = i;
		ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
		if (ret != RKNN_SUCC) {
			printf("rknn_query fail! ret=%d\n", ret);
			return -1;
		}
		printRKNNTensor(&(output_attrs[i]));
	}

	// Set Input Data
	rknn_input inputs[1];
	memset(inputs, 0, sizeof(inputs));
	inputs[0].index = 0;
	inputs[0].type = RKNN_TENSOR_UINT8;
	// inputs[0].size = img.cols*img.rows*img.channels() * BATCH_SIZE;
	inputs[0].size = 240 * 240 * 8 * BATCH_SIZE;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].buf = batch_img_data;

	ret = rknn_inputs_set(ctx, io_num.n_input, inputs);
	if (ret < 0) {
		printf("rknn_input_set fail! ret=%d\n", ret);
		return -1;
	}

	// Run
	printf("rknn_run\n");
	ret = rknn_run(ctx, nullptr);
	if (ret < 0) {
		printf("rknn_run fail! ret=%d\n", ret);
		return -1;
	}

	// Get Output
	rknn_output outputs[1];
	memset(outputs, 0, sizeof(outputs));
	outputs[0].want_float = 1;
	ret = rknn_outputs_get(ctx, 1, outputs, NULL);
	if (ret < 0) {
		printf("rknn_outputs_get fail! ret=%d\n", ret);
		return -1;
	}

	long stop = get_sys_time_interval();
	// stop_time = GetTickCount();
	printf("detect spend time--------:%ldms\n", stop - end_init);
	printf("end detect time:%lds\n", stop);

	vector<float> output(240 * 240);
	int leng = output_attrs[0].n_elems / BATCH_SIZE;
	// Post Process
	for (int i = 0; i < output_attrs[0].n_elems; i++) {

		float val = ((float*)(outputs[0].buf))[i];
		// printf("----->%d - %f\n", i, val);
		output[i] = val;
		// printf("size of ouput:%d\n", output.size());
	}

	// Release rknn_outputs
	rknn_outputs_release(ctx, 1, outputs);

	// Release
	if (ctx >= 0) {
		rknn_destroy(ctx);
	}
	if (model) {
		free(model);
	}

	printf("[1]:%f, [2]:%f, [3]:%f\n", output[0], output[1], output[2]);
	printf("======== Getting target done ========\n");
	pro_target(output, expand_type, mask, res_idx, res_idy);

}

// 最大池化函数
template <typename _Tp>
_Tp* MaxPolling::poll(_Tp* matrix, int matrix_w, int matrix_h, int kernel_size, int stride, bool show) {

	// 池化结果的size
	int result_w = (matrix_w - kernel_size) / stride + 1, result_h = (matrix_h - kernel_size) / stride + 1;
	// 申请内存
	_Tp* result = (_Tp*)malloc(sizeof(_Tp) * result_w * result_h);

	int x = 0, y = 0;
	for (int i = 0; i < result_h; i++) {
		for (int j = 0; j < result_w; j++) {
			result[y * result_w + x] = getMax(matrix, matrix_w, matrix_h, kernel_size, j * stride, i * stride);
			x++;
		}
		y++; x = 0;
	}

	if (show) {
		showMatrix(result, result_w, result_h);
	}

	return result;
}

template <typename _Tp>
void MaxPolling::showMatrix(_Tp matrix, int matrix_w, int matrix_h) {
	for (int i = 0; i < matrix_h; i++) {
		for (int j = 0; j < matrix_w; j++) {
			std::cout << matrix[i * matrix_w + j] << " ";
		}
		std::cout << std::endl;
	}
}

// 取kernel中最大值
template <typename _Tp>
_Tp MaxPolling::getMax(_Tp* matrix, int matrix_w, int matrix_h, int kernel_size, int x, int y) {
	int max_value = matrix[y * matrix_w + x];
	for (int i = 0; i < kernel_size; i++) {
		for (int j = 0; j < kernel_size; j++) {
			if (max_value < matrix[matrix_w * (y + i) + x + j]) {
				max_value = matrix[matrix_w * (y + i) + x + j];
			}
		}
	}
	return max_value;
}

void MaxPolling::testMaxPolling() {
	int matrix[36] = { 1,3,1,3,5,1,4,7,5,7,9,12,1,4,6,2,5,8,6,3,9,2,1,5,8,9,2,4,6,8,4,12,54,8,0,23 };
	poll(matrix, 6, 6, 2, 2, true);
}

bool RlApi::processTarget(vector<vector<double>> m_map, const int &idx, const int &idy, int &res_idx, int &res_idy) {

	// static vector<vector<double>> map(800, vector<double>(800, 0));
	printf("======== Start processing datas =======\n");
	// int size_x = 800;
	// int size_y = 800;
	// int size_x = m_map.getSizeX();
	// int size_y = m_map.getSizeY();
	int size_x = m_map.size();
	int size_y = m_map[0].size();
		
	int x_origin = 200;
	int y_origin = 200;
	double tmp_value;
	int robot__x;
	int robot__y;
	int expand_type;

	printf("Original Occupancy2DMaps::====> m_map.getCell(0,0)=%f, m_map.getCell(10,16)=%f, m_map.getCell(178,354)=%f\n", m_map[0][0], m_map[10][16], m_map[178][354]);
	// global map[0] = map occupancy: -1/100-->1(unexplored space/obstacles); 0-->0(free space) --- expand with 1
	// global map[1] = explored states: 0/100-->1(free space/obstacles); -1-->0(unexplored space) --- expand with 0
	// global pose = agent status on the map: 1-->robot position; 0-->No --- expand with 0
	// frontier map: 1-->frontiers; 0-->not frontiers --- expand with 0 before FBE: 1:explored spaces, 0:unexplored spaces
	// obstacles <-- 0.5 --> free space: threshold = [m_min_range, m_max_range]

	static vector<vector<double>> map_occupancy(1200, vector<double>(1200, 1));
	static vector<vector<double>> explored_states(1200, vector<double>(1200, 0));
	static vector<vector<double>> agent_status(1200, vector<double>(1200, 0));
	static vector<vector<double>> frontier_map(1200, vector<double>(1200, 0));


	printf("======== Getting maps data =======\n");
	// map_occupancy / explored_states / agent_status
	if (size_x == 800 && size_y == 800) {
		for (int x = 0; x < size_x; x++) {
			for (int y = 0; y < size_y; y++) {
				// tmp_value = m_map.getCell(x, y);
				tmp_value = m_map[y][x];
				// tmp_value = 0.4;
				// obstacles
				if (tmp_value <= m_min_range) {
					map_occupancy[x_origin + x][y_origin + y] = 1;
					explored_states[x_origin + x][y_origin + y] = 1;
				}
				// free space
				if (tmp_value >= m_max_range) {
					map_occupancy[x_origin + x][y_origin + y] = 0;
					explored_states[x_origin + x][y_origin + y] = 1;
				}
				// unexplored space
				if (tmp_value > m_min_range && tmp_value < m_max_range) {
					map_occupancy[x_origin + x][y_origin + y] = 1;
					explored_states[x_origin + x][y_origin + y] = 0;
				}

				// double float_value = map.getCell(x, y);
			}
		}
		robot__x = x_origin + idx;
		robot__y = y_origin + idy;
		agent_status[robot__x][robot__y] = 1;
		expand_type = 0;
	}
	if (size_x == 800 && size_y == 1200) {
		for (int x = 0; x < size_x; x++) {
			for (int y = 0; y < size_y; y++) {
				// tmp_value = m_map.getCell(x, y);
				tmp_value = m_map[y][x];
				// tmp_value = 0.4;
				// obstacles
				if (tmp_value <= m_min_range) {
					map_occupancy[x_origin + x][y] = 1;
					explored_states[x_origin + x][y] = 1;
				}
				// free space
				if (tmp_value >= m_max_range) {
					map_occupancy[x_origin + x][y] = 0;
					explored_states[x_origin + x][y] = 1;
				}
				// unexplored space
				if (tmp_value > m_min_range && tmp_value < m_max_range) {
					map_occupancy[x_origin + x][y] = 1;
					explored_states[x_origin + x][y] = 0;
				}

				// double float_value = map.getCell(x, y);
			}
		}
		robot__x = x_origin + idx;
		robot__y = idy;
		agent_status[robot__x][robot__y] = 1;
		expand_type = 1;
	}
	if (size_x == 1200 && size_y == 800) {
		for (int x = 0; x < size_x; x++) {
			for (int y = 0; y < size_y; y++) {
				// tmp_value = m_map.getCell(x, y);
				tmp_value = m_map[y][x];
				// tmp_value = 0.4;
				// obstacles
				if (tmp_value <= m_min_range) {
					map_occupancy[x][y_origin + y] = 1;
					explored_states[x][y_origin + y] = 1;
				}
				// free space
				if (tmp_value >= m_max_range) {
					map_occupancy[x][y_origin + y] = 0;
					explored_states[x][y_origin + y] = 1;
				}
				// unexplored space
				if (tmp_value > m_min_range && tmp_value < m_max_range) {
					map_occupancy[x][y_origin + y] = 1;
					explored_states[x][y_origin + y] = 0;
				}

				// double float_value = map.getCell(x, y);
			}
		}
		robot__x = idx;
		robot__y = y_origin + idy;
		agent_status[robot__x][robot__y] = 1;
		expand_type = 2;
	}
	if (size_x == 1200 && size_y == 1200) {
		for (int x = 0; x < size_x; x++) {
			for (int y = 0; y < size_y; y++) {
				// tmp_value = m_map.getCell(x, y);
				tmp_value = m_map[y][x];
				// tmp_value = 0.4;
				// obstacles
				if (tmp_value <= m_min_range) {
					map_occupancy[x][y] = 1;
					explored_states[x][y] = 1;
				}
				// free space
				if (tmp_value >= m_max_range) {
					map_occupancy[x][y] = 0;
					explored_states[x][y] = 1;
				}
				// unexplored space
				if (tmp_value > m_min_range && tmp_value < m_max_range) {
					map_occupancy[x][y] = 1;
					explored_states[x][y] = 0;
				}

				// double float_value = m_map.getCell(x, y);
			}
		}
		robot__x = idx;
		robot__y = idy;
		agent_status[robot__x][robot__y] = 1;
		expand_type = 3;
	}

	printf("======== Getting frontier maps =======\n");
	frontier_map = get_frontier(explored_states, map_occupancy, 1200, 1200);

	printf("======== Pooling & Cropping maps =======\n");
	double* Ocp_pooling;
	double* Expp_pooling;
	double* Agentp_pooling;
	double* Frontp_pooling;

	static vector<vector<double>> Ocp_crop(240, vector<double>(240));
	static vector<vector<double>> Expp_crop(240, vector<double>(240));
	static vector<vector<double>> Agentp_crop(240, vector<double>(240));
	static vector<vector<double>> Frontier_crop(240, vector<double>(240));

	static double Expmap[1440000];
	static double Ocmap[1440000];
	static double Agentmap[1200 * 1200];
	static double Frontmap[1200 * 1200];

	for (int i = 0; i < 1200; i++) {
		for (int j = 0; j < 1200; j++) {
			Ocmap[i * 1200 + j] = map_occupancy[i][j];
			Expmap[i * 1200 + j] = explored_states[i][j];
			Agentmap[i * 1200 + j] = agent_status[i][j];
			Frontmap[i * 1200 + j] = frontier_map[i][j];
		}
	}

	printf("======== Getting pooling maps =======\n");
	MaxPolling pool2d;
	// maps:(1200, 1200) ---> (240, 240)
	Ocp_pooling = pool2d.poll(Ocmap, 1200, 1200, 5, 5, false);
	Expp_pooling = pool2d.poll(Expmap, 1200, 1200, 5, 5, false);
	Agentp_pooling = pool2d.poll(Agentmap, 1200, 1200, 5, 5, false);
	Frontp_pooling = pool2d.poll(Frontmap, 1200, 1200, 5, 5, false);

	static vector<vector<double>> front_map_pool(240, vector<double>(240));
	for (int i = 0; i < 240; i++) {
		for (int j = 0; j < 240; j++) {
			front_map_pool[i][j] = Frontp_pooling[i * 240 + j];
		}
	}
	front_map_pool = filter_frontier(front_map_pool, 240, 240, 2);

	printf("======== Getting croped maps =======\n");
	// maps:(1200, 1200) ---> (240, 240)
	Ocp_crop = crop_map(map_occupancy, idx, idy, double(1));
	Expp_crop = crop_map(explored_states, idx, idy, double(0));
	Agentp_crop = crop_map(agent_status, idx, idy, double(0));
	Frontier_crop = crop_map(frontier_map, idx, idy, double(0));
	Frontier_crop = filter_frontier(Frontier_crop, 240, 240, 2);

	// double output_maps[8][240][240];
	static vector<vector<vector<double>>> output_maps(8, vector<vector<double>>(240, vector<double>(240)));
	for (int x = 0; x < 240; x++) {
		for (int y = 0; y < 240; y++) {
			output_maps[0][x][y] = Ocp_crop[x][y];
			output_maps[1][x][y] = Expp_crop[x][y];
			output_maps[2][x][y] = Agentp_crop[x][y];
			output_maps[3][x][y] = Frontier_crop[x][y];
			output_maps[4][x][y] = *(Ocp_pooling + x * 240 + y);
			output_maps[5][x][y] = *(Expp_pooling + x * 240 + y);
			output_maps[6][x][y] = *(Agentp_pooling + x * 240 + y);
			output_maps[7][x][y] = front_map_pool[x][y];
		}
	}
	printf("output_maps[0][0][0]=%f, output_maps[1][152][243]=%f, output_maps[4][123][21]=%f, output_maps[6][23][321]=%f\n", output_maps[0][0][0], output_maps[1][152][243], output_maps[4][123][21], output_maps[6][23][321]);
	printf("========== ALL DATA PREPARED ==========\n");

	int flag_end = 0;

	for (int i = 0; i < 240; i++) {
		for (int j = 0; j < 240; j++) {
			if (Frontier_crop[i][j] == 1) {
				flag_end = flag_end + 1;
			}
		}
	}

	if (flag_end == 0) {
		printf("There is no frontier left on the map\n");
	}

	printf("========== START PREDICTION ==========\n");
	vector<float> output_prob(240 * 240);
	predictions(output_maps, expand_type, Frontier_crop, res_idx, res_idy);

	return true;
}


vector<vector<double>> RlApi::get_inputs(int &robotx, int &roboty){
	printf("====== Generate random input data ======\n");
    // srand(time(0));
    static vector<vector<double>> inputs(800, vector<double>(800));

    for(int i=0; i<800; i++){
		for(int j=0;j<800;j++){
			inputs[i][j] = rand() * 1.0 / RAND_MAX;
		}
	}

	int robot_x = rand()%100+300;
	int robot_y = rand()%100+200;
	inputs[robot_x][robot_y] = 1;
	robotx = robot_x;
	roboty = robot_y;
	printf("====>robot_x=%d, robot_y=%d<====\n", robot_x, robot_y);
	printf("====> 800*800 random maps generated !!! <====\n");

    return inputs;
}



vector<vector<double>> RlApi::crop_map(vector<vector<double>> tmp, int x, int y, double padding_num) {
	vector<vector<double>> map(1200 + 240, vector<double>(1200 + 240, padding_num));
	vector<vector<double>> map_output(240, vector<double>(240));
	int robot_x = x + 120;
	int robot_y = y + 120;

	for (int i = 0; i < 1200; i++) {
		for (int j = 0; j < 1200; j++) {
			map[120 + i][120 + j] = tmp[i][j];
		}
	}

	for (int i = 0; i < 240; i++) {
		for (int j = 0; j < 240; j++) {
			map_output[i][j] = map[robot_y - 120 + i][robot_x - 120 + j];
		}
	}
	return map_output;
}

vector<vector<double>> RlApi::get_frontier(vector<vector<double>> explored_map,
										   vector<vector<double>> occupancy_map, int row, int column) 
{
	// global map[0] = map occupancy: -1/100-->1(unexplored space/obstacles); 0-->0(free space) --- expand with 1
	// global map[1] = explored states: 0/100-->1(free space/obstacles); -1-->0(unexplored space) --- expand with 0
	vector<vector<double>> map_frontier(1200, vector<double>(1200, 0));

	for (int i = 1; i < 1199; i++) {
		for (int j = 1; j < 1199; j++) {
			double tmp = explored_map[i][j - 1] + explored_map[i][j + 1] + explored_map[i - 1][j] + explored_map[i + 1][j];
			if (explored_map[i][j] == 0 && tmp != 0) {
				map_frontier[i][j] = 1;
			}
		}
	}

	for (int i = 0; i < 1200; i++) {
		for (int j = 0; j < 1200; j++) {
			map_frontier[i][j] = map_frontier[i][j] * occupancy_map[i][j];
		}
	}

	return map_frontier;
}

vector<vector<double>> RlApi::filter_frontier(vector<vector<double>> map, int row, int column, int minimum_size) {
	for (int i = minimum_size; i < row - minimum_size; i++) {
		for (int j = minimum_size; j < column - minimum_size; j++) {
			if (map[i][j] == 1) {
				double tmp = 0;
				for (int m = i - minimum_size; m < i + minimum_size + 1; m++) {
					for (int n = j - minimum_size; n < j + minimum_size + 1; n++) {
						tmp = tmp + map[m][n];
					}
				}
				if (tmp <= 2) {
					map[i][j] = 0;
				}
			}
		}
	}
	return map;
}

void RlApi::pro_target(vector<float> outputs, int expand_type, vector<vector<double>> mask, int &res_idx, int &res_idy) {

	cout << "===I am in pro_target func===" << endl;
	for (int i = 0; i < 240; i++) {
		for (int j = 0; j < 240; j++) {
			outputs[i * 240 + j] = outputs[i * 240 + j] * mask[i][j];
		}
	}

	double tmp_max = outputs[0];
	int tmp_index = 0;
	for (int i = 0; i < outputs.size(); i++) {
		if (tmp_max < outputs[i]) {
			tmp_max = outputs[i];
			tmp_index = i;
		}
	}

	vector<int> target_point(2);
	// x: rows;    y: columns
	// cout << "tmp_index = " << tmp_index << endl;
	int y = tmp_index % 240 * 5;
	int x = tmp_index / 240 * 5;

	// cout << "x=" << x << " y=" << y << endl;
	// transform target to original map
	if (expand_type == 0) {
		if ((x - 200) >= 0) {
			target_point[0] = x - 200;
		}
		if ((x - 200) < 0) {
			target_point[0] = x;
		}
		if ((y - 200) >= 0) {
			target_point[1] = y - 200;
		}
		if ((y - 200) < 0) {
			target_point[1] = y;
		}

	}
	if (expand_type == 1) {
		target_point[0] = x;
		if ((y - 200) >= 0) {
			target_point[1] = y - 200;
		}
		else {
			target_point[1] = y;
		}
	}
	if (expand_type == 2) {
		if ((x - 200) >= 0) {
			target_point[0] = x - 200;
		}
		else {
			target_point[0] = x;
		}
		target_point[1] = y;
	}
	if (expand_type == 3) {
		target_point[0] = x;
		target_point[1] = y;
	}

	/*
	// if transfomed target is obstacle --> use traditional algorithm
	if (m_map[y][x] <= m_min_range) {
		for (int i = x - 2; i < x + 3; i++) {
			double tmp_;
			for (int j = y - 2; j < y + 3; j++) {
				tmp_ = m_map[y][x];
				if (tmp_ > m_min_range) {
					target_point[0] = i;
					target_point[1] = j;
					break;
				}
			}
			if (tmp_ > m_min_range) {
				break;
			}
		}
		// use traditional way

	}*/
	res_idx = target_point[0];
	res_idy = target_point[1];

	printf("the target x is===> %d\n", res_idx);
	printf("the target y is===> %d\n", res_idy);

	cout << "====== get target done ======" << endl;
	// return target_point;
}


