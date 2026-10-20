#include "MockModel.h"

#include <thread>
#include <chrono>
#include <stdexcept>

InferenceResult simulateInfer(
    int task_id,
    const std::vector<float>& input
) {
    if (input.empty()) {
        throw std::invalid_argument("Empty input");
    }

    // 模拟推理耗时
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100)
    );

    float score = 0.0f;

    for (float x : input) {
        score += x;
    }

    return {task_id, score};
}