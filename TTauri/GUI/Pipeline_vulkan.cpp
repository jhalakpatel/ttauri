
#include "Pipeline_vulkan.hpp"

#include "Device_vulkan.hpp"
#include "Window_vulkan.hpp"

#include "TTauri/Logging.hpp"
#include "TTauri/utils.hpp"

#include <boost/assert.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace TTauri { namespace GUI {

using namespace std;

Pipeline_vulkan::Pipeline_vulkan(const std::shared_ptr<Window> &window) :
    Pipeline(window) {}

Pipeline_vulkan::~Pipeline_vulkan()
{
}

vk::Semaphore Pipeline_vulkan::render(uint32_t imageIndex, vk::Semaphore inputSemaphore)
{
    validateCommandBuffer(imageIndex);

    vk::Semaphore waitSemaphores[] = { inputSemaphore };

    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex] };

    vk::SubmitInfo submitInfo[] = { vk::SubmitInfo(1, waitSemaphores, waitStages, 1, &commandBuffers[imageIndex], 1, signalSemaphores) };

    device<Device_vulkan>()->graphicsQueue.submit(1, submitInfo, vk::Fence());

    return renderFinishedSemaphores[imageIndex];
}

void Pipeline_vulkan::buildShaders()
{
    shaderModules = createShaderModules();
    shaderStages = createShaderStages(shaderModules);
}

void Pipeline_vulkan::teardownShaders()
{
    auto vulkanDevice = device<Device_vulkan>();

    for (auto shaderModule : shaderModules) {
        vulkanDevice->intrinsic.destroy(shaderModule);
    }
    shaderModules.clear();
    shaderStages.clear();
}

void Pipeline_vulkan::buildVertexBuffers(size_t nrFrameBuffers)
{
    auto vulkanDevice = device<Device_vulkan>();

    vertexInputBindingDescription = createVertexInputBindingDescription();
    vertexInputAttributeDescriptions = createVertexInputAttributeDescriptions();

    vertexBuffers = createVertexBuffers(nrFrameBuffers, vertexInputBindingDescription.stride * maximumNumberOfVertices());

    auto memoryNeedsFlushingOffsetsAndSizes = vulkanDevice->allocateDeviceMemoryAndBind(vertexBuffers, vk::MemoryPropertyFlagBits::eHostVisible);
    vertexBufferMemory = get<0>(memoryNeedsFlushingOffsetsAndSizes);
    vertexBufferNeedsFlushing = get<1>(memoryNeedsFlushingOffsetsAndSizes);
    vertexBufferOffsets = get<2>(memoryNeedsFlushingOffsetsAndSizes);
    vertexBufferSizes = get<3>(memoryNeedsFlushingOffsetsAndSizes);
    vertexBufferDataSize = vertexBufferOffsets.back() + vertexBufferSizes.back();
    vertexBufferData = vulkanDevice->intrinsic.mapMemory(vertexBufferMemory, 0, vertexBufferDataSize, vk::MemoryMapFlags());
}

void Pipeline_vulkan::teardownVertexBuffers()
{
    auto vulkanDevice = device<Device_vulkan>();

    vulkanDevice->intrinsic.unmapMemory(vertexBufferMemory);
    vertexBufferData = nullptr;

    vulkanDevice->intrinsic.free(vertexBufferMemory);
    vertexBufferMemory = vk::DeviceMemory();

    for (auto buffer : vertexBuffers) {
        vulkanDevice->intrinsic.destroy(buffer);
    }
    vertexBuffers.clear();
    vertexBufferOffsets.clear();
    vertexBufferSizes.clear();
}

void Pipeline_vulkan::buildCommandBuffers(size_t nrFrameBuffers)
{
    auto vulkanDevice = device<Device_vulkan>();

    auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo(
        vulkanDevice->graphicsCommandPool, vk::CommandBufferLevel::ePrimary, boost::numeric_cast<uint32_t>(nrFrameBuffers));
    commandBuffers = vulkanDevice->intrinsic.allocateCommandBuffers(commandBufferAllocateInfo);

    commandBuffersValid.resize(nrFrameBuffers);
    invalidateCommandBuffers();
}

void Pipeline_vulkan::teardownCommandBuffers()
{
    auto vulkanDevice = device<Device_vulkan>();

    vulkanDevice->intrinsic.freeCommandBuffers(vulkanDevice->graphicsCommandPool, commandBuffers);
    commandBuffers.clear();
    commandBuffersValid.clear();
}

void Pipeline_vulkan::buildSemaphores(size_t nrFrameBuffers)
{
    auto vulkanDevice = device<Device_vulkan>();

    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    renderFinishedSemaphores.resize(nrFrameBuffers);
    for (size_t i = 0; i < nrFrameBuffers; i++) {
        renderFinishedSemaphores[i] = vulkanDevice->intrinsic.createSemaphore(semaphoreCreateInfo, nullptr);
    }
}

void Pipeline_vulkan::teardownSemaphores()
{
    auto vulkanDevice = device<Device_vulkan>();

    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        vulkanDevice->intrinsic.destroy(renderFinishedSemaphores[i]);
    }
    renderFinishedSemaphores.clear();
}

void Pipeline_vulkan::buildPipeline(vk::RenderPass _renderPass, vk::Extent2D extent)
{
    LOG_INFO("buildPipeline (%i, %i)") % extent.width % extent.height;

    renderPass = _renderPass;

    pipelineLayout = createPipelineLayout();

    pipelineVertexInputStateCreateInfo = createPipelineVertexInputStateCreateInfo(vertexInputBindingDescription, vertexInputAttributeDescriptions);

    pipelineInputAssemblyStateCreateInfo = createPipelineInputAssemblyStateCreateInfo();

    viewports = createViewports(extent);

    scissors = createScissors(extent);

    pipelineViewportStateCreateInfo = createPipelineViewportStateCreateInfo(viewports, scissors);

    pipelineRasterizationStateCreateInfo = createPipelineRasterizationStateCreateInfo();

    pipelineMultisampleStateCreateInfo = createPipelineMultisampleStateCreateInfo();

    pipelineColorBlendAttachmentStates = createPipelineColorBlendAttachmentStates();

    pipelineColorBlendStateCreateInfo = createPipelineColorBlendStateCreateInfo(pipelineColorBlendAttachmentStates);

    graphicsPipelineCreateInfo = {
        vk::PipelineCreateFlags(),
        boost::numeric_cast<uint32_t>(shaderStages.size()),
        shaderStages.data(),
        &pipelineVertexInputStateCreateInfo,
        &pipelineInputAssemblyStateCreateInfo,
        nullptr, // tesselationStateCreateInfo
        &pipelineViewportStateCreateInfo,
        &pipelineRasterizationStateCreateInfo,
        &pipelineMultisampleStateCreateInfo,
        nullptr, // pipelineDepthStencilCrateInfo
        &pipelineColorBlendStateCreateInfo,
        nullptr, // pipelineDynamicsStateCreateInfo
        pipelineLayout,
        renderPass,
        0, // subpass
        vk::Pipeline(), // basePipelineHandle
        -1 // basePipelineIndex
    };

    intrinsic = device<Device_vulkan>()->intrinsic.createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineCreateInfo);
}

void Pipeline_vulkan::teardownPipeline()
{
    auto vulkanDevice = device<Device_vulkan>();
    vulkanDevice->intrinsic.destroy(intrinsic);
    vulkanDevice->intrinsic.destroy(pipelineLayout);
}

void Pipeline_vulkan::buildForDeviceChange(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers)
{
    buildShaders();
    buildVertexBuffers(nrFrameBuffers);
    buildCommandBuffers(nrFrameBuffers);
    buildSemaphores(nrFrameBuffers);
    buildPipeline(renderPass, extent);
}

void Pipeline_vulkan::teardownForDeviceChange()
{
    teardownPipeline();
    teardownSemaphores();
    teardownCommandBuffers();
    teardownVertexBuffers();
    teardownShaders();
}

void Pipeline_vulkan::buildForSwapchainChange(vk::RenderPass renderPass, vk::Extent2D extent, size_t nrFrameBuffers)
{
    if (nrFrameBuffers != commandBuffers.size()) {
        teardownSemaphores();
        teardownCommandBuffers();
        teardownVertexBuffers();

        buildVertexBuffers(nrFrameBuffers);
        buildCommandBuffers(nrFrameBuffers);
        buildSemaphores(nrFrameBuffers);
    }
    invalidateCommandBuffers();
    buildPipeline(renderPass, extent);
}

void Pipeline_vulkan::teardownForSwapchainChange()
{
    teardownPipeline();
}

void Pipeline_vulkan::invalidateCommandBuffers()
{
    for (size_t imageIndex = 0; imageIndex < commandBuffersValid.size(); imageIndex++) {
        commandBuffersValid[imageIndex] = false;
    }
}

void Pipeline_vulkan::validateCommandBuffer(uint32_t imageIndex)
{
    if (commandBuffersValid[imageIndex]) {
        return;
    }

    LOG_INFO("validateCommandBuffer %i (%i, %i)") % imageIndex % scissors[0].extent.width % scissors[0].extent.height;

    commandBuffers[imageIndex].reset(vk::CommandBufferResetFlagBits::eReleaseResources);

    auto commandBufferBeginInfo = vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
    commandBuffers[imageIndex].begin(commandBufferBeginInfo);

    std::array<float, 4> blackColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    auto clearColor = vk::ClearValue(vk::ClearColorValue(blackColor));
    auto vulkanWindow = lock_dynamic_cast<Window_vulkan>(window);
    auto renderPassBeginInfo = vk::RenderPassBeginInfo(renderPass, vulkanWindow->swapchainFramebuffers[imageIndex], scissors[0], 1, &clearColor);
    commandBuffers[imageIndex].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffers[imageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, intrinsic);

    std::vector<vk::Buffer> tmpVertexBuffers = { vertexBuffers[imageIndex] };
    std::vector<vk::DeviceSize> tmpOffsets;
    for (size_t i = 0; i < tmpVertexBuffers.size(); i++) {
        tmpOffsets.push_back(0);
    }
    commandBuffers[imageIndex].bindVertexBuffers(0, tmpVertexBuffers, tmpOffsets);

    drawInCommandBuffer(commandBuffers[imageIndex]);

    commandBuffers[imageIndex].endRenderPass();

    commandBuffers[imageIndex].end();

    commandBuffersValid[imageIndex] = true;
}

vk::ShaderModule Pipeline_vulkan::loadShader(boost::filesystem::path path) const
{
    LOG_INFO("Loading shader %s") % path.filename().generic_string();

    auto tmp_path = path.generic_string();
    boost::interprocess::file_mapping mapped_file(tmp_path.c_str(), boost::interprocess::read_only);
    auto region = boost::interprocess::mapped_region(mapped_file, boost::interprocess::read_only);

    // Check uint32_t alignment of pointer.
    BOOST_ASSERT((reinterpret_cast<std::uintptr_t>(region.get_address()) & 3) == 0);

    auto shaderModuleCreateInfo =
        vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), region.get_size(), reinterpret_cast<uint32_t *>(region.get_address()));

    return device<Device_vulkan>()->intrinsic.createShaderModule(shaderModuleCreateInfo);
}

vk::PipelineLayout Pipeline_vulkan::createPipelineLayout() const
{
    auto pushConstantRanges = createPushConstantRanges();

    auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(), 0, nullptr, boost::numeric_cast<uint32_t>(pushConstantRanges.size()), pushConstantRanges.data());

    return device<Device_vulkan>()->intrinsic.createPipelineLayout(pipelineLayoutCreateInfo);
}

vk::PipelineVertexInputStateCreateInfo Pipeline_vulkan::createPipelineVertexInputStateCreateInfo(
    const vk::VertexInputBindingDescription &vertexBindingDescriptions,
    const std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) const
{
    return { vk::PipelineVertexInputStateCreateFlags(),
             1,
             &vertexBindingDescriptions,
             boost::numeric_cast<uint32_t>(vertexAttributeDescriptions.size()),
             vertexAttributeDescriptions.data() };
}

vk::PipelineInputAssemblyStateCreateInfo Pipeline_vulkan::createPipelineInputAssemblyStateCreateInfo() const
{
    return { vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE };
}

std::vector<vk::Viewport> Pipeline_vulkan::createViewports(vk::Extent2D extent) const
{
    return { { 0.0f, 0.0f, boost::numeric_cast<float>(extent.width), boost::numeric_cast<float>(extent.height), 0.0f, 1.0f } };
}

std::vector<vk::Rect2D> Pipeline_vulkan::createScissors(vk::Extent2D extent) const
{
    return { { { 0, 0 }, extent } };
}

vk::PipelineViewportStateCreateInfo
Pipeline_vulkan::createPipelineViewportStateCreateInfo(const std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors) const
{
    return { vk::PipelineViewportStateCreateFlags(),
             boost::numeric_cast<uint32_t>(viewports.size()),
             viewports.data(),
             boost::numeric_cast<uint32_t>(scissors.size()),
             scissors.data() };
}

vk::PipelineRasterizationStateCreateInfo Pipeline_vulkan::createPipelineRasterizationStateCreateInfo() const
{
    return {
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE, // depthClampEnable
        VK_FALSE, // rasterizerDiscardEnable
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        VK_FALSE, // depthBiasEnable
        0.0, // depthBiasConstantFactor
        0.0, // depthBiasClamp
        0.0, // depthBiasSlopeFactor
        1.0 // lineWidth
    };
}

vk::PipelineMultisampleStateCreateInfo Pipeline_vulkan::createPipelineMultisampleStateCreateInfo() const
{
    return {
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1,
        VK_FALSE, // sampleShadingEnable
        1.0f, // minSampleShading
        nullptr, // sampleMask
        VK_FALSE, // alphaToCoverageEnable
        VK_FALSE // alphaToOneEnable
    };
}

std::vector<vk::PipelineColorBlendAttachmentState> Pipeline_vulkan::createPipelineColorBlendAttachmentStates() const
{
    return { { VK_FALSE, // blendEnable
               vk::BlendFactor::eOne, // srcColorBlendFactor
               vk::BlendFactor::eZero, // dstColorBlendFactor
               vk::BlendOp::eAdd, // colorBlendOp
               vk::BlendFactor::eOne, // srcAlphaBlendFactor
               vk::BlendFactor::eZero, // dstAlphaBlendFactor
               vk::BlendOp::eAdd, // aphaBlendOp
               vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA } };
}

vk::PipelineColorBlendStateCreateInfo
Pipeline_vulkan::createPipelineColorBlendStateCreateInfo(const std::vector<vk::PipelineColorBlendAttachmentState> &attachements) const
{
    return { vk::PipelineColorBlendStateCreateFlags(),
             VK_FALSE, // logicOpenable
             vk::LogicOp::eCopy,
             boost::numeric_cast<uint32_t>(attachements.size()),
             attachements.data() };
}

std::vector<vk::Buffer> Pipeline_vulkan::createVertexBuffers(size_t nrBuffers, size_t bufferSize) const
{
    std::vector<vk::Buffer> buffers;
    auto vulkanDevice = device<Device_vulkan>();

    for (size_t i = 0; i < nrBuffers; i++) {
        vk::BufferCreateInfo vertexBufferCreateInfo = {
            vk::BufferCreateFlags(), bufferSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::SharingMode::eExclusive
        };

        auto buffer = vulkanDevice->intrinsic.createBuffer(vertexBufferCreateInfo, nullptr);
        buffers.push_back(buffer);
    }
    return buffers;
}

}}