#pragma once

struct StageConstants {

    int stageIndex = 1;
    std::string name = "Default";
    VkPushConstantRange pushConstantRange = {};
    uint32_t pushConstantsCount = 0;

    void setPushContantRange(VkShaderStageFlagBits stageFlagBits, uint32_t offset, uint32_t size) {
    pushConstantRange.stageFlags = stageFlagBits;
    pushConstantRange.offset = offset;
    pushConstantRange.size = size;
    }
};