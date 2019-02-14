//
//  Pipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <vulkan/vulkan.hpp>

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Device;

class Pipeline {
public:
    vk::Pipeline intrinsic;

    Device *device;
    
    boost::filesystem::path vertexShaderPath;
    boost::filesystem::path fragmentShaderPath;

    Pipeline(Device *device);
    virtual ~Pipeline();

    void initialize(vk::RenderPass renderPass, vk::Extent2D extent, vk::Format format);

protected:
    std::vector<vk::ShaderModule> shaderModules;
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    vk::PipelineLayout pipelineLayout;
    vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
    vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
    std::vector<vk::Viewport> viewports;
    std::vector<vk::Rect2D> scissors;
    vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo;
    vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo;
    vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;
    std::vector<vk::PipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates;
    vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo;
    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo;

    virtual vk::ShaderModule loadShader(boost::filesystem::path path) const;
    virtual std::vector<vk::ShaderModule> createShaderModules(void) const = 0;
    virtual std::vector<vk::PipelineShaderStageCreateInfo> createShaderStages(const std::vector<vk::ShaderModule> &shaders) const = 0;
    virtual vk::PipelineLayout createPipelineLayout(void) const;
    virtual vk::PipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(void) const;
    virtual vk::PipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyStateCreateInfo(void) const;
    virtual std::vector<vk::Viewport> createViewports(vk::Extent2D extent) const;
    virtual std::vector<vk::Rect2D> createScissors(vk::Extent2D extent) const;
    virtual vk::PipelineViewportStateCreateInfo createPipelineViewportStateCreateInfo(const std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors) const;
    virtual vk::PipelineRasterizationStateCreateInfo createPipelineRasterizationStateCreateInfo(void) const;
    virtual vk::PipelineMultisampleStateCreateInfo createPipelineMultisampleStateCreateInfo(void) const;
    virtual std::vector<vk::PipelineColorBlendAttachmentState> createPipelineColorBlendAttachmentStates(void) const;
    virtual vk::PipelineColorBlendStateCreateInfo createPipelineColorBlendStateCreateInfo(const std::vector<vk::PipelineColorBlendAttachmentState> &attachements) const;
};

}}}