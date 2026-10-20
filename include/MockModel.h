#pragma once

#include <vector>

struct InferenceResult {
    int task_id;
    float score;
};

InferenceResult simulateInfer(
    int task_id,
    const std::vector<float>& input
);