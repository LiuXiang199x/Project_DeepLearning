#include <RlApi/include/rl_api.h>

int main(){
	everest::planner::RlApi test;
	int robotxxx = 0;
	int robotyyy = 0;
	int result_x;
	int result_y;
	inputss = test.get_inputs(robotxxx, robotyyy);

	test.processTarget(inputss, robotxxx, robotyyy, result_x, result_y);
	// test.processTarget(400, 400, res_x, res_y);
	return 0;
}
