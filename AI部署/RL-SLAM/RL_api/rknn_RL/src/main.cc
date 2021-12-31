#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <string.h>
#include <uchar.h>
#include <COccupancyGridMap2D.h>
#include <numeric>
#include <algorithm>

#include "rknn_api.h"
#include "rl_api.h"

using namespace std;

//using namespace cv;

#define BATCH_SIZE 1
#define uchar unsigned char

#define img_width 64
#define img_height 64
#define img_channels 3

mrpt::slam::COccupancyGridMap2D maps_original;
// [-30, 15]

double min_range = 0.5 - 0.117;
double max_range = 0.5 + 0.059;
static vector<vector<int>> visited_map(1440, vector<int>(1440, 0));
static int Visitedmap[1200 * 1200] = {0};


bool processTarget(const int &idx, const int &idy, int &res_idx, int &res_idy);
int predictions(vector<vector<vector<int>>> inputs, int expand_type, vector<vector<int>> mask, int &res_idx, int &res_idy);
vector<vector<int>> crop_map(vector<vector<int>> tmp, int x, int y, int padding_num);
vector<vector<int>> get_frontier(int *explored_map, int *occupancy_map, int row, int column);
void pro_target(vector<float> outputs, int expand_type, vector<vector<int>> mask, int &res_idx, int &res_idy);


uint64_t time_tToTimestamp(const time_t &t ){
    return (((uint64_t)t) * (uint64_t)10000000) + ((uint64_t)116444736*1000000000);
}

uint64_t get_sys_time_interval(){
    timespec  tim;
    clock_gettime(CLOCK_MONOTONIC, &tim);
    return (time_tToTimestamp( tim.tv_sec ) + tim.tv_nsec/100)/10000;
}

static void printRKNNTensor(rknn_tensor_attr *attr) {
    printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n", 
            attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2], attr->dims[1], attr->dims[0], 
            attr->n_elems, attr->size, 0, attr->type, attr->qnt_type, attr->fl, attr->zp, attr->scale);
}

static unsigned char *load_model(const char *filename, int *model_size)
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

vector<vector<vector<double>>> get_inputs(){
    vector<vector<vector<double>>> inputs(8, vector<vector<double>>(240, vector<double>(240, 0)));
    return inputs;
}

int predictions(vector<vector<vector<int>>> inputs_map, int expand_type, vector<vector<int>> mask, int &res_idx, int &res_idy) {
	// const int img_width = 224;
	// const int img_height = 224;
	// const int img_channels = 3;
	printf("========== START GETTING TARGET ==========");
	rknn_context ctx;
	int ret;
	int model_len = 0;
	unsigned char* model;

	const char* model_path = "/userdata/ckpt.rknn";
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

	printf("===== load input data done =====");

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

class MaxPolling {
public:

	// 最大池化函数
	template <typename _Tp>
	_Tp* poll(_Tp* matrix, int matrix_w, int matrix_h, int kernel_size, int stride, bool show) {
		printf("======== Getting pooling maps =======\n");

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
	void showMatrix(_Tp matrix, int matrix_w, int matrix_h) {
		for (int i = 0; i < matrix_h; i++) {
			for (int j = 0; j < matrix_w; j++) {
				std::cout << matrix[i * matrix_w + j] << " ";
			}
			std::cout << std::endl;
		}
	}

	// 取kernel中最大值
	template <typename _Tp>
	_Tp getMax(_Tp* matrix, int matrix_w, int matrix_h, int kernel_size, int x, int y) {
		int max_value = 0;
		max_value = accumulate(matrix + matrix_w * (y + 0) + x + 0, matrix + matrix_w * (y + 0) + x + kernel_size, 0) + accumulate(matrix + matrix_w * (y + 1) + x + 0, matrix + matrix_w * (y + 1) + x + kernel_size, 0) + accumulate(matrix + matrix_w * (y + 2) + x + 0, matrix + matrix_w * (y + 2) + x + kernel_size, 0) + accumulate(matrix + matrix_w * (y + 3) + x + 0, matrix + matrix_w * (y + 3) + x + kernel_size, 0) + accumulate(matrix + matrix_w * (y + 4) + x + 0, matrix + matrix_w * (y + 4) + x + kernel_size, 0);
		
		for (int i = 0; i < kernel_size; i++) {
			for (int j = 0; j < kernel_size; j++) {
				if (max_value < matrix[matrix_w * (y + i) + x + j]) {
					max_value = matrix[matrix_w * (y + i) + x + j];
				}
			}
		}
		
		if(max_value==0){
			max_value = 0;
		}
		else{
			max_value = 1;
		}
		return max_value;
	}

	void testMaxPolling() {
		int matrix[36] = { 1,3,1,3,5,1,4,7,5,7,9,12,1,4,6,2,5,8,6,3,9,2,1,5,8,9,2,4,6,8,4,12,54,8,0,23 };
		poll(matrix, 6, 6, 2, 2, true);
	}
};

bool processTarget(const int &idx, const int &idy, int &res_idx, int &res_idy) {

	// static vector<vector<double>> map(800, vector<double>(800, 0));
	printf("======== Start processing datas =======");
	// int size_x = 800;
	// int size_y = 800;
	int size_x = maps_original.getSizeX();
	int size_y = maps_original.getSizeY();
	int x_origin = 200;
	int y_origin = 200;
	double tmp_value;
	int robot__x;
	int robot__y;
	int expand_type;


	// global map[0] = map occupancy: -1/100-->1(unexplored space/obstacles); 0-->0(free space) --- expand with 1
	// global map[1] = explored states: 0/100-->1(free space/obstacles); -1-->0(unexplored space) --- expand with 0
	// global pose = agent status on the map: 1-->robot position; 0-->No --- expand with 0
	// frontier map: 1-->frontiers; 0-->not frontiers --- expand with 0 before FBE: 1:explored spaces, 0:unexplored spaces
	// obstacles <-- 0.5 --> free space: threshold = [min_range, max_range]

	static vector<vector<int>> map_occupancy(1440, vector<int>(1440, 1));
	static vector<vector<int>> explored_states(1440, vector<int>(1440, 0));
	static vector<vector<int>> agent_status(1440, vector<int>(1440, 0));
	static vector<vector<int>> frontier_map(240, vector<int>(240, 0));
	static vector<vector<vector<int>>> output_maps(8, vector<vector<int>>(240, vector<int>(240)));	

	int* Ocp_pooling;
	int* Expp_pooling;
	int* Visitedmap_pooling;
	int* Agentp_pooling;

	static vector<vector<int>> Ocp_crop(240, vector<int>(240));
	static vector<vector<int>> Expp_crop(240, vector<int>(240));
	static vector<vector<int>> Visitedmap_crop(240, vector<int>(240));
	static vector<vector<int>> Agentp_crop(240, vector<int>(240));

	static int Ocmap[1440000];
	static int Expmap[1440000] = {0};
	static int Agentmap[1200 * 1200] = {0};
	// memset(Ocmap, 1, sizeof(int)*1440000);
	fill(Ocmap, Ocmap+1440000, 1);

	printf("======== Getting maps data =======");
	// map_occupancy / explored_states / agent_status
	if (size_x == 800 && size_y == 800) {
		for (int y = 0; y < size_y; y++) {
			for (int x = 0; x < size_x; x++) {
				tmp_value = maps_original.getCell(y, x);
				/// tmp_value = 0.4;
				// obstacles
				if (tmp_value <= min_range) {
					map_occupancy[y_origin+120+ y][x_origin+120+x] = 1;
					explored_states[y_origin+120 + y][x_origin+120 + x] = 1;
					Ocmap[(y_origin + y)*1200 + x_origin + x] = 1;
					Expmap[(y_origin + y)*1200 + x_origin + x] = 1;
				}
				// free space
				if (tmp_value >= max_range) {
					map_occupancy[y_origin+120 + y][x_origin+120 + x] = 0;
					explored_states[y_origin+120 + y][x_origin+120 + x] = 1;
					Ocmap[(y_origin + y)*1200 + x_origin + x] = 0;
					Expmap[(y_origin + y)*1200 + x_origin + x] = 1;
				}
				// unexplored space
				if (tmp_value > min_range && tmp_value < max_range) {
					map_occupancy[y_origin+120 + y][x_origin+120 + x] = 1;
					explored_states[y_origin+120 + y][x_origin+120 + x] = 0;
					Ocmap[(y_origin + y)*1200 + x_origin + x] = 1;
					Expmap[(y_origin + y)*1200 + x_origin + x] = 0;
				}

				// double float_value = map.getCell(x, y);
			}
		}
		robot__x = x_origin + idx;
		robot__y = y_origin + idy;
		agent_status[robot__y+120][robot__x+120] = 1;
		visited_map[robot__y+120][robot__x+120] = 1;
		Agentmap[robot__y*1200 + robot__x] = 1;
		Visitedmap[robot__y*1200 + robot__x] = 1;
		expand_type = 0;
	}
	if (size_x == 800 && size_y == 1200) {
		for (int y = 0; y < size_y; y++) {
			for (int x = 0; x < size_x; x++) {
				tmp_value = maps_original.getCell(y, x);
				// tmp_value = 0.4;
				// obstacles
				if (tmp_value <= min_range) {
					map_occupancy[y+120][x_origin+120 + x] = 1;
					explored_states[y+120][x_origin+120 + x] = 1;
					Ocmap[y*1200 + x_origin + x] = 1;
					Expmap[y*1200 + x_origin + x] = 1;
				}
				// free space
				if (tmp_value >= max_range) {
					map_occupancy[y+120][x_origin+120 + x] = 0;
					explored_states[y+120][x_origin+120 + x] = 1;
					Ocmap[y*1200 + x_origin + x] = 0;
					Expmap[y*1200 + x_origin + x] = 1;
				}
				// unexplored space
				if (tmp_value > min_range && tmp_value < max_range) {
					map_occupancy[y+120][x_origin+120 + x] = 1;
					explored_states[y+120][x_origin+120 + x] = 0;
					Ocmap[y*1200 + x_origin + x] = 1;
					Expmap[y*1200 + x_origin + x] = 0;
				}

				// double float_value = map.getCell(x, y);
			}
		}
		robot__x = x_origin + idx;
		robot__y = idy;
		agent_status[robot__y+120][robot__x+120] = 1;
		visited_map[robot__y+120][robot__x+120] = 1;
		Agentmap[robot__y*1200 + robot__x] = 1;
		Visitedmap[robot__y*1200 + robot__x] = 1;
		expand_type = 1;
	}
	if (size_x == 1200 && size_y == 800) {
		for (int y = 0; y < size_y; y++) {
			for (int x = 0; x < size_x; x++) {
				tmp_value = maps_original.getCell(y, x);
				// tmp_value = 0.4;
				// obstacles
				if (tmp_value <= min_range) {
					map_occupancy[y_origin +120+ y][x+120] = 1;
					explored_states[y_origin +120+ y][x+120] = 1;
					Ocmap[(y_origin + y)*1200 + x] = 1;
					Expmap[(y_origin + y)*1200 + x] = 1;
				}
				// free space
				if (tmp_value >= max_range) {
					map_occupancy[y_origin + y+120][x+120] = 0;
					explored_states[y_origin + y+120][x+120] = 1;
					Ocmap[(y_origin + y)*1200 + x] = 0;
					Expmap[(y_origin + y)*1200 + x] = 1;
				}
				// unexplored space
				if (tmp_value > min_range && tmp_value < max_range) {
					map_occupancy[y_origin + y+120][x+120] = 1;
					explored_states[y_origin + y+120][x+120] = 0;
					Ocmap[(y_origin + y)*1200 + x] = 1;
					Expmap[(y_origin + y)*1200 + x] = 0;
				}

				// double float_value = map.getCell(x, y);
			}
		}
		robot__x = idx;
		robot__y = y_origin + idy;
		agent_status[robot__y+120][robot__x+120] = 1;
		visited_map[robot__y+120][robot__x+120] = 1;
		Agentmap[robot__y*1200 + robot__x] = 1;
		Visitedmap[robot__y*1200 + robot__x] = 1;
		expand_type = 2;
	}
	if (size_x == 1200 && size_y == 1200) {
		for (int y = 0; y < size_y; y++) {
			for (int x = 0; x < size_x; x++) {
				tmp_value = maps_original.getCell(y, x);
				// tmp_value = 0.4;
				// obstacles
				if (tmp_value <= min_range) {
					map_occupancy[x+120][y+120] = 1;
					explored_states[x+120][y+120] = 1;
					Ocmap[y*1200 + x] = 1;
					Expmap[y*1200 + x] = 1;
				}
				// free space
				if (tmp_value >= max_range) {
					map_occupancy[x+120][y+120] = 0;
					explored_states[x+120][y+120] = 1;
					Ocmap[y*1200 + x] = 0;
					Expmap[y*1200 + x] = 1;
				}
				// unexplored space
				if (tmp_value > min_range && tmp_value < max_range) {
					map_occupancy[x+120][y+120] = 1;
					explored_states[x+120][y+120] = 0;
					Ocmap[y*1200 + x] = 1;
					Expmap[y*1200 + x] = 0;
				}

				// double float_value = maps_original.getCell(x, y);
			}
		}
		robot__x = idx;
		robot__y = idy;
		agent_status[robot__y+120][robot__x+120] = 1;
		visited_map[robot__y+120][robot__x+120] = 1;
		Agentmap[robot__y*1200 + robot__x] = 1;
		Visitedmap[robot__y*1200 + robot__x] = 1;
		expand_type = 3;
	}


	printf("======== Pooling & Cropping maps =======\n");
	// timeval start_maxpoolmaps, end_maxpoolmaps, start_cropmaps, end_cropmaps;
	MaxPolling pool2d;
	// maps:(1200, 1200) ---> (240, 240)
	Ocp_pooling = pool2d.poll(Ocmap, 1200, 1200, 5, 5, false);
	Expp_pooling = pool2d.poll(Expmap, 1200, 1200, 5, 5, false);
	Agentp_pooling = pool2d.poll(Agentmap, 1200, 1200, 5, 5, false);
	Visitedmap_pooling = pool2d.poll(Visitedmap, 1200, 1200, 5, 5, false);
	frontier_map = get_frontier(Expp_pooling, Ocp_pooling, 240, 240);


	/// =========== Getting croped maps ===========
	// maps:(1200, 1200) ---> (240, 240)
	Ocp_crop = crop_map(map_occupancy, robot__x+120, robot__y+120, int(1));
	Expp_crop = crop_map(explored_states, robot__x+120, robot__y+120, int(0));
	Agentp_crop = crop_map(agent_status, robot__x+120, robot__y+120, int(0));
	Visitedmap_crop = crop_map(visited_map, robot__x+120, robot__y+120, int(0));
	cout << "Ocp_cropppp:::::::" << Ocp_crop.size() << endl;

	// double output_maps[8][240][240];
	printf("======== Reshape all datas to (8,G,G) =======\n");
	// timeval start_model_input, end_model_input;
	// double output_maps[8][240][240];

	output_maps[0] = Ocp_crop;
	output_maps[1] = Expp_crop;
	output_maps[2] = Visitedmap_crop;
	output_maps[3] = Agentp_crop;
	
	for (int x = 0; x < 240; x++) {
		for (int y = 0; y < 240; y++) {
			output_maps[4][x][y] = *(Ocp_pooling + x * 240 + y);
			output_maps[5][x][y] = *(Expp_pooling + x * 240 + y);
			output_maps[6][x][y] = *(Visitedmap_pooling + x * 240 + y);
			output_maps[7][x][y] = *(Agentp_pooling + x * 240 + y);
		}
	}
	printf("========== ALL DATA PREPARED ==========\n");
	printf("========== START PREDICTION ==========\n");
	static vector<float> output_prob(240 * 240);
	predictions(output_maps, expand_type, frontier_map, res_idx, res_idy);

	return true;
}

vector<vector<int>> crop_map(vector<vector<int>> tmp, int x, int y, int padding_num) {
	printf("======== Getting croped maps =======\n");
	// static vector<vector<double>> map(1200 + 240, vector<double>(1200 + 240, padding_num));
	static vector<vector<int>> map_output(240, vector<int>(240, padding_num));

	for (int i = 0; i < 240; i++) {
		map_output[i].assign(tmp[i+y-120].begin()+x-120, tmp[i+y-120].begin()+x+120);
	}
	return map_output;
}

// input: explored_map=>[240*240]
vector<vector<int>> get_frontier(int *explored_map, int *occupancy_map, int row, int column) {
	printf("======== Getting frontier maps =======\n");
	// global map[0] = map occupancy: -1/100-->1(unexplored space/obstacles); 0-->0(free space) --- expand with 1
	// global map[1] = explored states: 0/100-->1(free space/obstacles); -1-->0(unexplored space) --- expand with 0
	static vector<vector<int>> map_frontier(row, vector<int>(column, 0));

	for (int i = 1; i < row-1; i++) {
		for (int j = 1; j < column-1; j++) {
			int tmp = *(explored_map+(i-1)*240+j) + *(explored_map+(i+1)*240+j) +*(explored_map+i*240+j+1) +*(explored_map+i*240+j-1);

			if (*(explored_map+240*i + j) == 1) {
				if (*(explored_map+(i-1)*240+j) ==0  or *(explored_map+(i+1)*240+j) ==0 or *(explored_map+i*240+j+1) ==0 or *(explored_map+i*240+j-1)==0){
					if(tmp!=0){
						map_frontier[i][j] = 1;
					}
				}
			}
			if(map_frontier[i][j]==1 && *(occupancy_map+240*i + j)==0){
					map_frontier[i][j] = 1;
			}
			if(map_frontier[i][j]==1 && *(occupancy_map+240*i + j)==1){
					map_frontier[i][j] = 0;
			}
		}
	}
	for (int i = 1; i < row-1; i++) {
		for (int j = 1; j < column-1; j++) {
			int tmp_ = map_frontier[i][j - 1] + map_frontier[i][j + 1] + map_frontier[i - 1][j] + map_frontier[i + 1][j];
			if(tmp_==0){
				map_frontier[i][j]=0;
			}
		}
	}
	/*
	for(int i=0; i<240;i++){
		for(int j=0; j<240;j++){
			cout << map_frontier[i][j] << "";
		}
		cout << endl;
	}*/
	return map_frontier;
}

void pro_target(vector<float> outputs, int expand_type, vector<vector<int>> mask, int &res_idx, int &res_idy) {

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

	int x = tmp_index % 240 * 5;
	int y = tmp_index / 240 * 5;

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

	
	// if transfomed target is obstacle --> use traditional algorithm
	if (maps_original.getCell(y, x) <= min_range) {
		for (int i = y - 2; i < y + 3; i++) {
			double tmp_;
			for (int j = x - 2; j < x + 3; j++) {
				tmp_ = maps_original.getCell(i, j);
				if (tmp_ > min_range) {
					target_point[0] = j;
					target_point[1] = i;
					break;
				}
			}
			if (tmp_ > min_range) {
				break;
			}
		}
		// use traditional way

	}
	res_idx = target_point[0];
	res_idy = target_point[1];

	cout << "====== get target done ======" << endl;
	// return target_point;
}

int main() {

	return 0;
}
