//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "misc/callbacks.h"
#include "misc/debug.h"
#include "wrappers/buffer.h"
#include "wrappers/command_buffer.h"
#include "wrappers/command_pool.h"
#include "wrappers/compute_pipeline_manager.h"
#include "wrappers/descriptor_set.h"
#include "wrappers/device.h"
#include "wrappers/event.h"
#include "wrappers/framebuffer.h"
#include "wrappers/graphics_pipeline_manager.h"
#include "wrappers/image.h"
#include "wrappers/pipeline_layout.h"
#include "wrappers/query_pool.h"
#include "wrappers/render_pass.h"


/* Command stashing should be enabled by default for builds that care. */
bool Anvil::CommandBufferBase::m_command_stashing_disabled = false;


/** Please see header for specification */
Anvil::CommandBufferBase::BeginQueryCommand::BeginQueryCommand(std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                               Anvil::QueryIndex                 in_entry,
                                                               VkQueryControlFlags               in_flags)
    :Command(COMMAND_TYPE_BEGIN_QUERY)
{
    entry          = in_entry;
    flags          = in_flags;
    query_pool_ptr = in_query_pool_ptr;
}

/** Please see header for specification */
Anvil::BeginRenderPassCommand::BeginRenderPassCommand(uint32_t                            in_n_clear_values,
                                                      const VkClearValue*                 in_clear_value_ptrs,
                                                      std::shared_ptr<Anvil::Framebuffer> in_fbo_ptr,
                                                      VkRect2D                            in_render_area,
                                                      std::shared_ptr<Anvil::RenderPass>  in_render_pass_ptr,
                                                      VkSubpassContents                   in_contents)
    :Command(COMMAND_TYPE_BEGIN_RENDER_PASS)
{
    contents        = in_contents;
    fbo_ptr         = in_fbo_ptr;
    render_area     = in_render_area;
    render_pass_ptr = in_render_pass_ptr;

    for (uint32_t n_clear_value = 0;
                  n_clear_value < in_n_clear_values;
                ++n_clear_value)
    {
        clear_values.push_back(in_clear_value_ptrs[n_clear_value]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindDescriptorSetsCommand::BindDescriptorSetsCommand(VkPipelineBindPoint                    in_pipeline_bind_point,
                                                                               std::shared_ptr<Anvil::PipelineLayout> in_layout_ptr,
                                                                               uint32_t                               in_first_set,
                                                                               uint32_t                               in_set_count,
                                                                               std::shared_ptr<Anvil::DescriptorSet>* in_descriptor_set_ptrs,
                                                                               uint32_t                               in_dynamic_offset_count,
                                                                               const uint32_t*                        in_dynamic_offset_ptrs)
    :Command(COMMAND_TYPE_BIND_DESCRIPTOR_SETS)
{
    first_set           = in_first_set;
    layout_ptr          = in_layout_ptr;
    pipeline_bind_point = in_pipeline_bind_point;

    for (uint32_t n_set = 0;
                  n_set < in_set_count;
                ++n_set)
    {
        descriptor_sets.push_back(in_descriptor_set_ptrs[n_set]);
    }

    for (uint32_t n_dynamic_offset = 0;
                  n_dynamic_offset < in_dynamic_offset_count;
                ++n_dynamic_offset)
    {
        dynamic_offsets.push_back(in_dynamic_offset_ptrs[n_dynamic_offset]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindIndexBufferCommand::BindIndexBufferCommand(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                                         VkDeviceSize                   in_offset,
                                                                         VkIndexType                    in_index_type)
    :Command(COMMAND_TYPE_BIND_INDEX_BUFFER)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    index_type = in_index_type;
    offset     = in_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindPipelineCommand::BindPipelineCommand(VkPipelineBindPoint in_pipeline_bind_point,
                                                                   Anvil::PipelineID   in_pipeline_id)
    :Command(COMMAND_TYPE_BIND_PIPELINE)
{
    pipeline_bind_point = in_pipeline_bind_point;
    pipeline_id         = in_pipeline_id;
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindVertexBuffersCommand::BindVertexBuffersCommand(uint32_t                        in_start_binding,
                                                                             uint32_t                        in_binding_count,
                                                                             std::shared_ptr<Anvil::Buffer>* in_buffer_ptrs,
                                                                             const VkDeviceSize*             in_offset_ptrs)
    :Command(COMMAND_TYPE_BIND_VERTEX_BUFFER)
{
    start_binding = in_start_binding;
    
    for (uint32_t n_binding = 0;
                  n_binding < in_binding_count;
                ++n_binding)
    {
        bindings.push_back(BindVertexBuffersCommandBinding(in_buffer_ptrs[n_binding],
                                                           in_offset_ptrs[n_binding]) );
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindVertexBuffersCommandBinding::BindVertexBuffersCommandBinding(const BindVertexBuffersCommandBinding& in)
{
    buffer     = in.buffer;
    buffer_ptr = in.buffer_ptr;
    offset     = in.offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindVertexBuffersCommandBinding::BindVertexBuffersCommandBinding(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                                                           VkDeviceSize                   in_offset)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    offset     = in_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::BlitImageCommand::BlitImageCommand(std::shared_ptr<Anvil::Image> in_src_image_ptr,
                                                             VkImageLayout                 in_src_image_layout,
                                                             std::shared_ptr<Anvil::Image> in_dst_image_ptr,
                                                             VkImageLayout                 in_dst_image_layout,
                                                             uint32_t                      in_region_count,
                                                             const VkImageBlit*            in_region_ptrs,
                                                             VkFilter                      in_filter)
    :Command(COMMAND_TYPE_BLIT_IMAGE)
{
    dst_image        = in_dst_image_ptr->get_image();
    dst_image_layout = in_dst_image_layout;
    dst_image_ptr    = in_dst_image_ptr;
    filter           = in_filter;
    src_image        = in_src_image_ptr->get_image();
    src_image_layout = in_src_image_layout;
    src_image_ptr    = in_src_image_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::ClearAttachmentsCommand::ClearAttachmentsCommand(uint32_t                 in_n_attachments,
                                                                           const VkClearAttachment* in_attachments,
                                                                           uint32_t                 in_n_rects,
                                                                           const VkClearRect*       in_rect_ptrs)
    :Command(COMMAND_TYPE_CLEAR_ATTACHMENTS)
{
    for (uint32_t n_attachment = 0;
                  n_attachment < in_n_attachments;
                ++n_attachment)
    {
        attachments.push_back(ClearAttachmentsCommandAttachment(in_attachments[n_attachment].aspectMask,
                                                                in_attachments[n_attachment].clearValue,
                                                                in_attachments[n_attachment].colorAttachment) );
    }

    for (uint32_t n_rect = 0;
                  n_rect < in_n_rects;
                ++n_rect)
    {
        rects.push_back(in_rect_ptrs[n_rect]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::ClearColorImageCommand::ClearColorImageCommand(std::shared_ptr<Anvil::Image>  in_image_ptr,
                                                                         VkImageLayout                  in_image_layout,
                                                                         const VkClearColorValue*       in_color_ptr,
                                                                         uint32_t                       in_range_count,
                                                                         const VkImageSubresourceRange* in_range_ptrs)
    :Command(COMMAND_TYPE_CLEAR_COLOR_IMAGE)
{
    color        = *in_color_ptr;
    image        = in_image_ptr->get_image();
    image_layout = in_image_layout;
    image_ptr    = in_image_ptr;

    for (uint32_t n_range = 0;
                  n_range < in_range_count;
                ++n_range)
    {
        ranges.push_back(in_range_ptrs[n_range]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::ClearDepthStencilImageCommand::ClearDepthStencilImageCommand(std::shared_ptr<Anvil::Image>   in_image_ptr,
                                                                                       VkImageLayout                   in_image_layout,
                                                                                       const VkClearDepthStencilValue* in_depth_stencil_ptr,
                                                                                       uint32_t                        in_range_count,
                                                                                       const VkImageSubresourceRange*  in_range_ptrs)
    :Command(COMMAND_TYPE_CLEAR_DEPTH_STENCIL_IMAGE)
{
    depth_stencil = *in_depth_stencil_ptr;
    image         =  in_image_ptr->get_image();
    image_layout  =  in_image_layout;
    image_ptr     =  in_image_ptr;

    for (uint32_t n_range = 0;
                  n_range < in_range_count;
                ++n_range)
    {
        ranges.push_back(in_range_ptrs[n_range]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyBufferCommand::CopyBufferCommand(std::shared_ptr<Anvil::Buffer> in_src_buffer_ptr,
                                                               std::shared_ptr<Anvil::Buffer> in_dst_buffer_ptr,
                                                               uint32_t                       in_region_count,
                                                               const VkBufferCopy*            in_region_ptrs)
    :Command(COMMAND_TYPE_COPY_BUFFER)
{
    dst_buffer     = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr = in_dst_buffer_ptr;
    src_buffer     = in_src_buffer_ptr->get_buffer();
    src_buffer_ptr = in_src_buffer_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyBufferToImageCommand::CopyBufferToImageCommand(std::shared_ptr<Anvil::Buffer> in_src_buffer_ptr,
                                                                             std::shared_ptr<Anvil::Image>  in_dst_image_ptr,
                                                                             VkImageLayout                  in_dst_image_layout,
                                                                             uint32_t                       in_region_count,
                                                                             const VkBufferImageCopy*       in_region_ptrs)
    :Command(COMMAND_TYPE_COPY_BUFFER_TO_IMAGE)
{
    dst_image        = in_dst_image_ptr->get_image();
    dst_image_layout = in_dst_image_layout;
    dst_image_ptr    = in_dst_image_ptr;
    src_buffer       = in_src_buffer_ptr->get_buffer();
    src_buffer_ptr   = in_src_buffer_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyImageCommand::CopyImageCommand(std::shared_ptr<Anvil::Image> in_src_image_ptr,
                                                             VkImageLayout                 in_src_image_layout,
                                                             std::shared_ptr<Anvil::Image> in_dst_image_ptr,
                                                             VkImageLayout                 in_dst_image_layout,
                                                             uint32_t                      in_region_count,
                                                             const VkImageCopy*            in_region_ptrs)
    :Command(COMMAND_TYPE_COPY_IMAGE)
{
    dst_image        = in_dst_image_ptr->get_image();
    dst_image_layout = in_dst_image_layout;
    dst_image_ptr    = in_dst_image_ptr;
    src_image        = in_src_image_ptr->get_image();
    src_image_layout = in_src_image_layout;
    src_image_ptr    = in_src_image_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyImageToBufferCommand::CopyImageToBufferCommand(std::shared_ptr<Anvil::Image>  in_src_image_ptr,
                                                                             VkImageLayout                  in_src_image_layout,
                                                                             std::shared_ptr<Anvil::Buffer> in_dst_buffer_ptr,
                                                                             uint32_t                       in_region_count,
                                                                             const VkBufferImageCopy*       in_region_ptrs)
    :Command(COMMAND_TYPE_COPY_IMAGE_TO_BUFFER)
{
    dst_buffer       = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr   = in_dst_buffer_ptr;
    src_image        = in_src_image_ptr->get_image();
    src_image_layout = in_src_image_layout;
    src_image_ptr    = in_src_image_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyQueryPoolResultsCommand::CopyQueryPoolResultsCommand(std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                                                   Anvil::QueryIndex                 in_start_query,
                                                                                   uint32_t                          in_query_count,
                                                                                   std::shared_ptr<Anvil::Buffer>    in_dst_buffer_ptr,
                                                                                   VkDeviceSize                      in_dst_offset,
                                                                                   VkDeviceSize                      in_dst_stride,
                                                                                   VkQueryResultFlags                in_flags)
    :Command(COMMAND_TYPE_COPY_QUERY_POOL_RESULTS)
{
    dst_buffer     = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr = in_dst_buffer_ptr;
    dst_offset     = in_dst_offset;
    dst_stride     = in_dst_stride;
    flags          = in_flags;
    query_count    = in_query_count;
    query_pool_ptr = in_query_pool_ptr;
    start_query    = in_start_query;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DispatchCommand::DispatchCommand(uint32_t in_x,
                                                           uint32_t in_y,
                                                           uint32_t in_z)
    :Command(COMMAND_TYPE_DISPATCH)
{
    x = in_x;
    y = in_y;
    z = in_z;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DispatchIndirectCommand::DispatchIndirectCommand(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                                           VkDeviceSize                   in_offset)
    :Command(COMMAND_TYPE_DISPATCH_INDIRECT)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    offset     = in_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawCommand::DrawCommand(uint32_t in_vertex_count,
                                                   uint32_t in_instance_count,
                                                   uint32_t in_first_vertex,
                                                   uint32_t in_first_instance)
    :Command(COMMAND_TYPE_DRAW)
{
    first_instance = in_first_instance;
    first_vertex   = in_first_vertex;
    instance_count = in_instance_count;
    vertex_count   = in_vertex_count;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndexedCommand::DrawIndexedCommand(uint32_t in_index_count,
                                                                 uint32_t in_instance_count,
                                                                 uint32_t in_first_index,
                                                                 int32_t  in_vertex_offset,
                                                                 uint32_t in_first_instance)
    :Command(COMMAND_TYPE_DRAW_INDEXED)
{
    first_index    = in_first_index;
    first_instance = in_first_instance;
    index_count    = in_index_count;
    instance_count = in_instance_count;
    vertex_offset  = in_vertex_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndexedIndirectCommand::DrawIndexedIndirectCommand(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                                                 VkDeviceSize                   in_offset,
                                                                                 uint32_t                       in_draw_count,
                                                                                 uint32_t                       in_stride)
    :Command(COMMAND_TYPE_DRAW_INDEXED_INDIRECT)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    draw_count = in_draw_count;
    offset     = in_offset;
    stride     = in_stride;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndexedIndirectCountAMDCommand::DrawIndexedIndirectCountAMDCommand(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                                                                 VkDeviceSize                   in_offset,
                                                                                                 std::shared_ptr<Anvil::Buffer> in_count_buffer_ptr,
                                                                                                 VkDeviceSize                   in_count_offset,
                                                                                                 uint32_t                       in_max_draw_count,
                                                                                                 uint32_t                       in_stride)
    :Command(COMMAND_TYPE_DRAW_INDEXED_INDIRECT_COUNT_AMD)
{
    buffer           = in_buffer_ptr->get_buffer();
    buffer_ptr       = in_buffer_ptr;
    count_buffer     = in_count_buffer_ptr->get_buffer();
    count_buffer_ptr = in_count_buffer_ptr;
    count_offset     = in_count_offset;
    max_draw_count   = in_max_draw_count;
    stride           = in_stride;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndirectCommand::DrawIndirectCommand(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                                   VkDeviceSize                   in_offset,
                                                                   uint32_t                       in_count,
                                                                   uint32_t                       in_stride)
    :Command(COMMAND_TYPE_DRAW_INDIRECT)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    count      = in_count;
    offset     = in_offset;
    stride     = in_stride;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndirectCountAMDCommand::DrawIndirectCountAMDCommand(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                                                   VkDeviceSize                   in_offset,
                                                                                   std::shared_ptr<Anvil::Buffer> in_count_buffer_ptr,
                                                                                   VkDeviceSize                   in_count_offset,
                                                                                   uint32_t                       in_max_draw_count,
                                                                                   uint32_t                       in_stride)
    :Command(COMMAND_TYPE_DRAW_INDIRECT_COUNT_AMD)
{
    buffer           = in_buffer_ptr->get_buffer();
    buffer_ptr       = in_buffer_ptr;
    count_buffer     = in_count_buffer_ptr->get_buffer();
    count_buffer_ptr = in_count_buffer_ptr;
    count_offset     = in_count_offset;
    max_draw_count   = in_max_draw_count;
    stride           = in_stride;
}


/** Please see header for specification */
Anvil::CommandBufferBase::EndQueryCommand::EndQueryCommand(std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                           Anvil::QueryIndex                 in_entry)
    :Command(COMMAND_TYPE_END_QUERY)
{
    entry          = in_entry;
    query_pool_ptr = in_query_pool_ptr;
}

/** Please see header for specification */
Anvil::EndRenderPassCommand::EndRenderPassCommand()
    :Command(COMMAND_TYPE_END_RENDER_PASS)
{
}

/** Please see header for specification */
Anvil::CommandBufferBase::ExecuteCommandsCommand::ExecuteCommandsCommand(uint32_t                                        in_cmd_buffers_count,
                                                                         std::shared_ptr<Anvil::SecondaryCommandBuffer>* in_cmd_buffer_ptrs)
    :Command(COMMAND_TYPE_EXECUTE_COMMANDS)
{
    for (uint32_t n_cmd_buffer = 0;
                  n_cmd_buffer < in_cmd_buffers_count;
                ++n_cmd_buffer)
    {
        command_buffers.push_back    (in_cmd_buffer_ptrs[n_cmd_buffer]->get_command_buffer() );
        command_buffer_ptrs.push_back(in_cmd_buffer_ptrs[n_cmd_buffer]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::FillBufferCommand::FillBufferCommand(std::shared_ptr<Anvil::Buffer> in_dst_buffer_ptr,
                                                               VkDeviceSize                   in_dst_offset,
                                                               VkDeviceSize                   in_size,
                                                               uint32_t                       in_data)
    :Command(COMMAND_TYPE_FILL_BUFFER)
{
    data           = in_data;
    dst_buffer     = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr = in_dst_buffer_ptr;
    dst_offset     = in_dst_offset;
    size           = in_size;
}

/** Please see header for specification */
Anvil::CommandBufferBase::NextSubpassCommand::NextSubpassCommand(VkSubpassContents in_contents)
    :Command(COMMAND_TYPE_NEXT_SUBPASS)
{
    contents = in_contents;
}

/** Please see header for specification */
Anvil::PipelineBarrierCommand::PipelineBarrierCommand(VkPipelineStageFlags       in_src_stage_mask,
                                                      VkPipelineStageFlags       in_dst_stage_mask,
                                                      VkBool32                   in_by_region,
                                                      uint32_t                   in_memory_barrier_count,
                                                      const MemoryBarrier* const in_memory_barrier_ptr_ptr,
                                                      uint32_t                   in_buffer_memory_barrier_count,
                                                      const BufferBarrier* const in_buffer_memory_barrier_ptr_ptr,
                                                      uint32_t                   in_image_memory_barrier_count,
                                                      const ImageBarrier*  const in_image_memory_barrier_ptr_ptr)
    :Command(COMMAND_TYPE_PIPELINE_BARRIER)
{
    by_region      = in_by_region;
    dst_stage_mask = static_cast<VkPipelineStageFlagBits>(in_dst_stage_mask);
    src_stage_mask = static_cast<VkPipelineStageFlagBits>(in_src_stage_mask);

    for (uint32_t n_buffer_memory_barrier = 0;
                  n_buffer_memory_barrier < in_buffer_memory_barrier_count;
                ++n_buffer_memory_barrier)
    {
        buffer_barriers.push_back(in_buffer_memory_barrier_ptr_ptr[n_buffer_memory_barrier]);
    }

    for (uint32_t n_image_memory_barrier = 0;
                  n_image_memory_barrier < in_image_memory_barrier_count;
                ++n_image_memory_barrier)
    {
        image_barriers.push_back(in_image_memory_barrier_ptr_ptr[n_image_memory_barrier]);
    }

    for (uint32_t n_memory_barrier = 0;
                  n_memory_barrier < in_memory_barrier_count;
                ++n_memory_barrier)
    {
        memory_barriers.push_back(in_memory_barrier_ptr_ptr[n_memory_barrier]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::PushConstantsCommand::PushConstantsCommand(std::shared_ptr<Anvil::PipelineLayout> in_layout_ptr,
                                                                     VkShaderStageFlags                     in_stage_flags,
                                                                     uint32_t                               in_offset,
                                                                     uint32_t                               in_size,
                                                                     const void*                            in_values)
    :Command(COMMAND_TYPE_PUSH_CONSTANTS)
{
    layout_ptr  = in_layout_ptr;
    offset      = in_offset;
    size        = in_size;
    stage_flags = in_stage_flags;
    values      = in_values;
}

/** Please see header for specification */
Anvil::CommandBufferBase::ResetEventCommand::ResetEventCommand(std::shared_ptr<Anvil::Event> in_event_ptr,
                                                               VkPipelineStageFlags          in_stage_mask)
    :Command(COMMAND_TYPE_RESET_EVENT)
{
    event      = in_event_ptr->get_event();
    event_ptr  = in_event_ptr;
    stage_mask = in_stage_mask;
}

/** Please see header for specification */
Anvil::CommandBufferBase::ResetQueryPoolCommand::ResetQueryPoolCommand(std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                                       Anvil::QueryIndex                 in_start_query,
                                                                       uint32_t                          in_query_count)
    :Command(COMMAND_TYPE_RESET_QUERY_POOL)
{
    query_count    = in_query_count;
    query_pool_ptr = in_query_pool_ptr;
    start_query    = in_start_query;
}

/** Please see header for specification */
Anvil::CommandBufferBase::ResolveImageCommand::ResolveImageCommand(std::shared_ptr<Anvil::Image> in_src_image_ptr,
                                                                   VkImageLayout                 in_src_image_layout,
                                                                   std::shared_ptr<Anvil::Image> in_dst_image_ptr,
                                                                   VkImageLayout                 in_dst_image_layout,
                                                                   uint32_t                      in_region_count,
                                                                   const VkImageResolve*         in_region_ptrs)
    :Command(COMMAND_TYPE_RESOLVE_IMAGE)
{
    dst_image        = in_dst_image_ptr->get_image();
    dst_image_layout = in_dst_image_layout;
    dst_image_ptr    = in_dst_image_ptr;
    src_image        = in_src_image_ptr->get_image();
    src_image_layout = in_src_image_layout;
    src_image_ptr    = in_src_image_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetBlendConstantsCommand::SetBlendConstantsCommand(const float in_blend_constants[4])
    :Command(COMMAND_TYPE_SET_BLEND_CONSTANTS)
{
    memcpy(blend_constants,
           in_blend_constants,
           sizeof(float) * 4);
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetDepthBiasCommand::SetDepthBiasCommand(float in_depth_bias_constant_factor,
                                                                   float in_depth_bias_clamp,
                                                                   float in_slope_scaled_depth_bias)
    :Command(COMMAND_TYPE_SET_DEPTH_BIAS)
{
    depth_bias_clamp           = in_depth_bias_clamp;
    depth_bias_constant_factor = in_depth_bias_constant_factor;
    slope_scaled_depth_bias    = in_slope_scaled_depth_bias;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetDepthBoundsCommand::SetDepthBoundsCommand(float in_min_depth_bounds,
                                                                       float in_max_depth_bounds)
    :Command(COMMAND_TYPE_SET_DEPTH_BOUNDS)
{
    max_depth_bounds = in_max_depth_bounds;
    min_depth_bounds = in_min_depth_bounds;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetEventCommand::SetEventCommand(std::shared_ptr<Anvil::Event> in_event_ptr,
                                                           VkPipelineStageFlags          in_stage_mask)
    :Command(COMMAND_TYPE_SET_EVENT)
{
    event      = in_event_ptr->get_event();
    event_ptr  = in_event_ptr;
    stage_mask = in_stage_mask;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetLineWidthCommand::SetLineWidthCommand(float in_line_width)
    :Command(COMMAND_TYPE_SET_LINE_WIDTH)
{
    line_width = in_line_width;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetScissorCommand::SetScissorCommand(uint32_t        in_first_scissor,
                                                               uint32_t        in_scissor_count,
                                                               const VkRect2D* in_scissor_ptrs)
    :Command(COMMAND_TYPE_SET_SCISSOR)
{
    first_scissor = in_first_scissor;

    for (uint32_t n_scissor = 0;
                  n_scissor < in_scissor_count;
                ++n_scissor)
    {
        scissors.push_back(in_scissor_ptrs[n_scissor]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetStencilCompareMaskCommand::SetStencilCompareMaskCommand(VkStencilFaceFlags in_face_mask,
                                                                                     uint32_t           in_stencil_compare_mask)
    :Command(COMMAND_TYPE_SET_STENCIL_COMPARE_MASK)
{
    face_mask            = in_face_mask;
    stencil_compare_mask = in_stencil_compare_mask;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetStencilReferenceCommand::SetStencilReferenceCommand(VkStencilFaceFlags in_face_mask,
                                                                                 uint32_t           in_stencil_reference)
    :Command(COMMAND_TYPE_SET_STENCIL_REFERENCE)
{
    face_mask         = in_face_mask;
    stencil_reference = in_stencil_reference;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetStencilWriteMaskCommand::SetStencilWriteMaskCommand(VkStencilFaceFlags in_face_mask,
                                                                                 uint32_t           in_stencil_write_mask)
    :Command(COMMAND_TYPE_SET_STENCIL_WRITE_MASK)
{
    face_mask          = in_face_mask;
    stencil_write_mask = in_stencil_write_mask;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetViewportCommand::SetViewportCommand(uint32_t          in_first_viewport,
                                                                 uint32_t          in_viewport_count,
                                                                 const VkViewport* in_viewport_ptrs)
    :Command(COMMAND_TYPE_SET_VIEWPORT)
{
    first_viewport = in_first_viewport;

    for (uint32_t n_viewport = 0;
                  n_viewport < in_viewport_count;
                ++n_viewport)
    {
        viewports.push_back(in_viewport_ptrs[n_viewport]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::UpdateBufferCommand::UpdateBufferCommand(std::shared_ptr<Anvil::Buffer> in_dst_buffer_ptr,
                                                                   VkDeviceSize                   in_dst_offset,
                                                                   VkDeviceSize                   in_data_size,
                                                                   const uint32_t*                in_data_ptr)
    :Command(COMMAND_TYPE_UPDATE_BUFFER)
{
    data_ptr       = in_data_ptr;
    data_size      = in_data_size;
    dst_buffer     = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr = in_dst_buffer_ptr;
    dst_offset     = in_dst_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::WaitEventsCommand::WaitEventsCommand(uint32_t                       in_event_count,
                                                               std::shared_ptr<Anvil::Event>* in_event_ptrs,
                                                               VkPipelineStageFlags           in_src_stage_mask,
                                                               VkPipelineStageFlags           in_dst_stage_mask,
                                                               uint32_t                       in_memory_barrier_count,
                                                               const MemoryBarrier* const     in_memory_barriers_ptr,
                                                               uint32_t                       in_buffer_memory_barrier_count,
                                                               const BufferBarrier* const     in_buffer_memory_barriers_ptr,
                                                               uint32_t                       in_image_memory_barrier_count,
                                                               const ImageBarrier* const      in_image_memory_barriers_ptr)
    :Command(COMMAND_TYPE_WAIT_EVENTS)
{
    dst_stage_mask = static_cast<VkPipelineStageFlagBits>(in_dst_stage_mask);
    src_stage_mask = static_cast<VkPipelineStageFlagBits>(in_src_stage_mask);

    for (uint32_t n_event = 0;
                  n_event < in_event_count;
                ++n_event)
    {
        events.push_back    (in_event_ptrs[n_event]->get_event() );
        event_ptrs.push_back(in_event_ptrs[n_event]);
    }

    for (uint32_t n_buffer_memory_barrier = 0;
                  n_buffer_memory_barrier < in_buffer_memory_barrier_count;
                ++n_buffer_memory_barrier)
    {
        buffer_barriers.push_back(in_buffer_memory_barriers_ptr[n_buffer_memory_barrier]);
    }

    for (uint32_t n_image_memory_barrier = 0;
                  n_image_memory_barrier < in_image_memory_barrier_count;
                ++n_image_memory_barrier)
    {
        image_barriers.push_back(in_image_memory_barriers_ptr[n_image_memory_barrier]);
    }

    for (uint32_t n_memory_barrier = 0;
                  n_memory_barrier < in_memory_barrier_count;
                ++n_memory_barrier)
    {
        memory_barriers.push_back(in_memory_barriers_ptr[n_memory_barrier]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::WriteTimestampCommand::WriteTimestampCommand(VkPipelineStageFlagBits           in_pipeline_stage,
                                                                       std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                                       Anvil::QueryIndex                 in_entry)
    :Command(COMMAND_TYPE_WRITE_TIMESTAMP)
{
    entry          = in_entry;
    pipeline_stage = in_pipeline_stage;
    query_pool_ptr = in_query_pool_ptr;
}


/** Constructor.
 *
 *  @param device_ptr              Device to use.
 *  @param parent_command_pool_ptr Command pool to allocate the commands from. Must not be nullptr.
 *  @param type                    Command buffer type
 **/
Anvil::CommandBufferBase::CommandBufferBase(std::weak_ptr<Anvil::Device>        device_ptr,
                                            std::shared_ptr<Anvil::CommandPool> parent_command_pool_ptr,
                                            Anvil::CommandBufferType            type)
    :CallbacksSupportProvider(COMMAND_BUFFER_CALLBACK_ID_COUNT),
     m_command_buffer         (VK_NULL_HANDLE),
     m_device_ptr             (device_ptr),
     m_is_renderpass_active   (false),
     m_parent_command_pool_ptr(parent_command_pool_ptr),
     m_recording_in_progress  (false),
     m_type                   (type)
{
    anvil_assert(parent_command_pool_ptr != nullptr);
}

/** Destructor.
 *
 *  Releases the underlying Vulkan command buffer instance.
 *
 *  Throws an assertion failure if recording is in progress.
 **/
Anvil::CommandBufferBase::~CommandBufferBase()
{
    anvil_assert(!m_recording_in_progress);

    if (m_command_buffer                    != VK_NULL_HANDLE &&
        m_parent_command_pool_ptr.expired() == false)
    {
        std::shared_ptr<Anvil::CommandPool> command_pool_locked_ptr(m_parent_command_pool_ptr);
        std::shared_ptr<Anvil::Device>      device_locked_ptr      (m_device_ptr);

        /* Unregister the command buffer from the pool */
        command_pool_locked_ptr->on_command_buffer_wrapper_destroyed(this);

        /* Physically free the command buffer we own */
        vkFreeCommandBuffers(device_locked_ptr->get_device_vk(),
                             command_pool_locked_ptr->get_command_pool(),
                             1, /* commandBufferCount */
                            &m_command_buffer);

        m_command_buffer = VK_NULL_HANDLE;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        clear_commands();
    }
    #endif
}

#ifdef STORE_COMMAND_BUFFER_COMMANDS
    /** Clears the command vector by releasing all command descriptors back to the heap memory. */
    void Anvil::CommandBufferBase::clear_commands()
    {
        m_commands.clear();
    }
#endif

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_begin_query(std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                  Anvil::QueryIndex                 in_entry,
                                                  VkQueryControlFlags               in_flags)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BeginQueryCommand(in_query_pool_ptr,
                                                   in_entry,
                                                   in_flags) );
        }
    }
    #endif

    vkCmdBeginQuery(m_command_buffer,
                    in_query_pool_ptr->get_query_pool(),
                    in_entry,
                    in_flags);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_bind_descriptor_sets(VkPipelineBindPoint                    in_pipeline_bind_point,
                                                           std::shared_ptr<Anvil::PipelineLayout> in_layout_ptr,
                                                           uint32_t                               in_first_set,
                                                           uint32_t                               in_set_count,
                                                           std::shared_ptr<Anvil::DescriptorSet>* in_descriptor_set_ptrs,
                                                           uint32_t                               in_dynamic_offset_count,
                                                           const uint32_t*                        in_dynamic_offset_ptrs)
{
    /* Note: Command supported inside and outside the renderpass. */
    VkDescriptorSet dss_vk[16];
    bool            result = false;

    anvil_assert(in_set_count < sizeof(dss_vk) / sizeof(dss_vk[0]) );

    for (uint32_t n_set = 0;
                  n_set < in_set_count;
                ++n_set)
    {
        dss_vk[n_set] = in_descriptor_set_ptrs[n_set]->get_descriptor_set_vk();
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BindDescriptorSetsCommand(in_pipeline_bind_point,
                                                           in_layout_ptr,
                                                           in_first_set,
                                                           in_set_count,
                                                           in_descriptor_set_ptrs,
                                                           in_dynamic_offset_count,
                                                           in_dynamic_offset_ptrs) );
        }
    }
    #endif

    vkCmdBindDescriptorSets(m_command_buffer,
                            in_pipeline_bind_point,
                            in_layout_ptr->get_pipeline_layout(),
                            in_first_set,
                            in_set_count,
                            dss_vk,
                            in_dynamic_offset_count,
                            in_dynamic_offset_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_bind_index_buffer(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                        VkDeviceSize                   in_offset,
                                                        VkIndexType                    in_index_type)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BindIndexBufferCommand(in_buffer_ptr,
                                                        in_offset,
                                                        in_index_type) );
        }
    }
    #endif

    vkCmdBindIndexBuffer(m_command_buffer,
                         in_buffer_ptr->get_buffer(),
                         in_offset,
                         in_index_type);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_bind_pipeline(VkPipelineBindPoint in_pipeline_bind_point,
                                                    Anvil::PipelineID   in_pipeline_id)
{
    /* Command supported inside and outside the renderpass. */
    std::shared_ptr<Anvil::Device> device_locked_ptr(m_device_ptr);
    VkPipeline                     pipeline_vk = VK_NULL_HANDLE;
    bool                           result      = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    pipeline_vk = (in_pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE) ? device_locked_ptr->get_compute_pipeline_manager()->get_compute_pipeline  (in_pipeline_id)
                                                                             : device_locked_ptr->get_graphics_pipeline_manager()->get_graphics_pipeline(in_pipeline_id);

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BindPipelineCommand(in_pipeline_bind_point,
                                                     in_pipeline_id) );
        }
    }
    #endif

    vkCmdBindPipeline(m_command_buffer,
                      in_pipeline_bind_point,
                      pipeline_vk);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_bind_vertex_buffers(uint32_t                        in_start_binding,
                                                          uint32_t                        in_binding_count,
                                                          std::shared_ptr<Anvil::Buffer>* in_buffer_ptrs,
                                                          const VkDeviceSize*             in_offset_ptrs)
{
    /* Note: Command supported inside and outside the renderpass. */
    VkBuffer buffers[16];
    bool     result = false;

    anvil_assert(in_binding_count < sizeof(buffers) / sizeof(buffers[0]) );

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BindVertexBuffersCommand(in_start_binding,
                                                          in_binding_count,
                                                          in_buffer_ptrs,
                                                          in_offset_ptrs) );
        }
    }
    #endif

    for (uint32_t n_binding = 0;
                  n_binding < in_binding_count;
                ++n_binding)
    {
        buffers[n_binding] = in_buffer_ptrs[n_binding]->get_buffer();
    }

    vkCmdBindVertexBuffers(m_command_buffer,
                           in_start_binding,
                           in_binding_count,
                           buffers,
                           in_offset_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_blit_image(std::shared_ptr<Anvil::Image> in_src_image_ptr,
                                                 VkImageLayout                 in_src_image_layout,
                                                 std::shared_ptr<Anvil::Image> in_dst_image_ptr,
                                                 VkImageLayout                 in_dst_image_layout,
                                                 uint32_t                      in_region_count,
                                                 const VkImageBlit*            in_region_ptrs,
                                                 VkFilter                      in_filter)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BlitImageCommand(in_src_image_ptr,
                                                  in_src_image_layout,
                                                  in_dst_image_ptr,
                                                  in_dst_image_layout,
                                                  in_region_count,
                                                  in_region_ptrs,
                                                  in_filter) );
        }
    }
    #endif

    vkCmdBlitImage(m_command_buffer,
                   in_src_image_ptr->get_image(),
                   in_src_image_layout,
                   in_dst_image_ptr->get_image(),
                   in_dst_image_layout,
                   in_region_count,
                   in_region_ptrs,
                   in_filter);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_clear_attachments(uint32_t                 in_n_attachments,
                                                        const VkClearAttachment* in_attachment_ptrs,
                                                        uint32_t                 in_n_rects,
                                                        const VkClearRect*       in_rect_ptrs)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ClearAttachmentsCommand(in_n_attachments,
                                                         in_attachment_ptrs,
                                                         in_n_rects,
                                                         in_rect_ptrs) );
        }
    }
    #endif

    vkCmdClearAttachments(m_command_buffer,
                          in_n_attachments,
                          in_attachment_ptrs,
                          in_n_rects,
                          in_rect_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_clear_color_image(std::shared_ptr<Anvil::Image>  in_image_ptr,
                                                        VkImageLayout                  in_image_layout,
                                                        const VkClearColorValue*       in_color_ptr,
                                                        uint32_t                       in_range_count,
                                                        const VkImageSubresourceRange* in_range_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ClearColorImageCommand(in_image_ptr,
                                                        in_image_layout,
                                                        in_color_ptr,
                                                        in_range_count,
                                                        in_range_ptrs) );
        }
    }
    #endif

    vkCmdClearColorImage(m_command_buffer,
                         in_image_ptr->get_image(),
                         in_image_layout,
                         in_color_ptr,
                         in_range_count,
                         in_range_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_clear_depth_stencil_image(std::shared_ptr<Anvil::Image>   in_image_ptr,
                                                                VkImageLayout                   in_image_layout,
                                                                const VkClearDepthStencilValue* in_depth_stencil_ptr,
                                                                uint32_t                        in_range_count,
                                                                const VkImageSubresourceRange*  in_range_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ClearDepthStencilImageCommand(in_image_ptr,
                                                               in_image_layout,
                                                               in_depth_stencil_ptr,
                                                               in_range_count,
                                                               in_range_ptrs) );
        }
    }
    #endif

    vkCmdClearDepthStencilImage(m_command_buffer,
                                in_image_ptr->get_image(),
                                in_image_layout,
                                in_depth_stencil_ptr,
                                in_range_count,
                                in_range_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_buffer(std::shared_ptr<Anvil::Buffer> in_src_buffer_ptr,
                                                  std::shared_ptr<Anvil::Buffer> in_dst_buffer_ptr,
                                                  uint32_t                       in_region_count,
                                                  const VkBufferCopy*            in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyBufferCommand(in_src_buffer_ptr,
                                                   in_dst_buffer_ptr,
                                                   in_region_count,
                                                   in_region_ptrs) );
        }
    }
    #endif

    vkCmdCopyBuffer(m_command_buffer,
                    in_src_buffer_ptr->get_buffer(),
                    in_dst_buffer_ptr->get_buffer(),
                    in_region_count,
                    in_region_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_buffer_to_image(std::shared_ptr<Anvil::Buffer> in_src_buffer_ptr,
                                                           std::shared_ptr<Anvil::Image>  in_dst_image_ptr,
                                                           VkImageLayout                  in_dst_image_layout,
                                                           uint32_t                       in_region_count,
                                                           const VkBufferImageCopy*       in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyBufferToImageCommand(in_src_buffer_ptr,
                                                          in_dst_image_ptr,
                                                          in_dst_image_layout,
                                                          in_region_count,
                                                          in_region_ptrs) );
        }
    }
    #endif

    vkCmdCopyBufferToImage(m_command_buffer,
                           in_src_buffer_ptr->get_buffer(),
                           in_dst_image_ptr->get_image(),
                           in_dst_image_layout,
                           in_region_count,
                           in_region_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_image(std::shared_ptr<Anvil::Image> in_src_image_ptr,
                                                 VkImageLayout                 in_src_image_layout,
                                                 std::shared_ptr<Anvil::Image> in_dst_image_ptr,
                                                 VkImageLayout                 in_dst_image_layout,
                                                 uint32_t                      in_region_count,
                                                 const VkImageCopy*            in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyImageCommand(in_src_image_ptr,
                                                  in_src_image_layout,
                                                  in_dst_image_ptr,
                                                  in_dst_image_layout,
                                                  in_region_count,
                                                  in_region_ptrs) );
        }
    }
    #endif

    vkCmdCopyImage(m_command_buffer,
                   in_src_image_ptr->get_image(),
                   in_src_image_layout,
                   in_dst_image_ptr->get_image(),
                   in_dst_image_layout,
                   in_region_count,
                   in_region_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_image_to_buffer(std::shared_ptr<Anvil::Image>  in_src_image_ptr,
                                                           VkImageLayout                  in_src_image_layout,
                                                           std::shared_ptr<Anvil::Buffer> in_dst_buffer_ptr,
                                                           uint32_t                       in_region_count,
                                                           const VkBufferImageCopy*       in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyImageToBufferCommand(in_src_image_ptr,
                                                          in_src_image_layout,
                                                          in_dst_buffer_ptr,
                                                          in_region_count,
                                                          in_region_ptrs) );
        }
    }
    #endif

    vkCmdCopyImageToBuffer(m_command_buffer,
                           in_src_image_ptr->get_image(),
                           in_src_image_layout,
                           in_dst_buffer_ptr->get_buffer(),
                           in_region_count,
                           in_region_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_query_pool_results(std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                              Anvil::QueryIndex                 in_start_query,
                                                              uint32_t                          in_query_count,
                                                              std::shared_ptr<Anvil::Buffer>    in_dst_buffer_ptr,
                                                              VkDeviceSize                      in_dst_offset,
                                                              VkDeviceSize                      in_dst_stride,
                                                              VkQueryResultFlags                in_flags)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyQueryPoolResultsCommand(in_query_pool_ptr,
                                                             in_start_query,
                                                             in_query_count,
                                                             in_dst_buffer_ptr,
                                                             in_dst_offset,
                                                             in_dst_stride,
                                                             in_flags) );
        }
    }
    #endif

    vkCmdCopyQueryPoolResults(m_command_buffer,
                              in_query_pool_ptr->get_query_pool(),
                              in_start_query,
                              in_query_count,
                              in_dst_buffer_ptr->get_buffer(),
                              in_dst_offset,
                              in_dst_stride,
                              in_flags);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_dispatch(uint32_t in_x,
                                               uint32_t in_y,
                                               uint32_t in_z)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DispatchCommand(in_x,
                                                 in_y,
                                                 in_z) );
        }
    }
    #endif

    vkCmdDispatch(m_command_buffer,
                  in_x,
                  in_y,
                  in_z);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_dispatch_indirect(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                        VkDeviceSize                   in_offset)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DispatchIndirectCommand(in_buffer_ptr,
                                                         in_offset) );
        }
    }
    #endif

    vkCmdDispatchIndirect(m_command_buffer,
                          in_buffer_ptr->get_buffer(),
                          in_offset);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw(uint32_t in_vertex_count,
                                           uint32_t in_instance_count,
                                           uint32_t in_first_vertex,
                                           uint32_t in_first_instance)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawCommand(in_vertex_count,
                                             in_instance_count,
                                             in_first_vertex,
                                             in_first_instance) );
        }
    }
    #endif

    vkCmdDraw(m_command_buffer,
              in_vertex_count,
              in_instance_count,
              in_first_vertex,
              in_first_instance);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indexed(uint32_t in_index_count,
                                                   uint32_t in_instance_count,
                                                   uint32_t in_first_index,
                                                   int32_t  in_vertex_offset,
                                                   uint32_t in_first_instance)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndexedCommand(in_index_count,
                                                    in_instance_count,
                                                    in_first_index,
                                                    in_vertex_offset,
                                                    in_first_instance) );
        }
    }
    #endif

    vkCmdDrawIndexed(m_command_buffer,
                     in_index_count,
                     in_instance_count,
                     in_first_index,
                     in_vertex_offset,
                     in_first_instance);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indexed_indirect(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                            VkDeviceSize                   in_offset,
                                                            uint32_t                       in_count,
                                                            uint32_t                       in_stride)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndexedIndirectCommand(in_buffer_ptr,
                                                            in_offset,
                                                            in_count,
                                                            in_stride) );
        }
    }
    #endif

    vkCmdDrawIndexedIndirect(m_command_buffer,
                             in_buffer_ptr->get_buffer(),
                             in_offset,
                             in_count,
                             in_stride);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indexed_indirect_count_AMD(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                                      VkDeviceSize                   in_offset,
                                                                      std::shared_ptr<Anvil::Buffer> in_count_buffer_ptr,
                                                                      VkDeviceSize                   in_count_offset,
                                                                      uint32_t                       in_max_draw_count,
                                                                      uint32_t                       in_stride)
{
    Anvil::ExtensionAMDDrawIndirectCountEntrypoints entrypoints;
    bool                                            result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndexedIndirectCountAMDCommand(in_buffer_ptr,
                                                                    in_offset,
                                                                    in_count_buffer_ptr,
                                                                    in_count_offset,
                                                                    in_max_draw_count,
                                                                    in_stride) );
        }
    }
    #endif

    entrypoints = m_device_ptr.lock()->get_extension_amd_draw_indirect_count_entrypoints();
    
    entrypoints.vkCmdDrawIndexedIndirectCountAMD(m_command_buffer,
                                                 in_buffer_ptr->get_buffer(),
                                                 in_offset,
                                                 in_count_buffer_ptr->get_buffer(),
                                                 in_count_offset,
                                                 in_max_draw_count,
                                                 in_stride);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indirect(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                    VkDeviceSize                   in_offset,
                                                    uint32_t                       in_count,
                                                    uint32_t                       in_stride)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndirectCommand(in_buffer_ptr,
                                                     in_offset,
                                                     in_count,
                                                     in_stride) );
        }
    }
    #endif

    vkCmdDrawIndirect(m_command_buffer,
                      in_buffer_ptr->get_buffer(),
                      in_offset,
                      in_count,
                      in_stride);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indirect_count_AMD(std::shared_ptr<Anvil::Buffer> in_buffer_ptr,
                                                              VkDeviceSize                   in_offset,
                                                              std::shared_ptr<Anvil::Buffer> in_count_buffer_ptr,
                                                              VkDeviceSize                   in_count_offset,
                                                              uint32_t                       in_max_draw_count,
                                                              uint32_t                       in_stride)
{
    Anvil::ExtensionAMDDrawIndirectCountEntrypoints entrypoints;
    bool                                            result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndirectCountAMDCommand(in_buffer_ptr,
                                                             in_offset,
                                                             in_count_buffer_ptr,
                                                             in_count_offset,
                                                             in_max_draw_count,
                                                             in_stride) );
        }
    }
    #endif

    entrypoints = m_device_ptr.lock()->get_extension_amd_draw_indirect_count_entrypoints();

    entrypoints.vkCmdDrawIndirectCountAMD(m_command_buffer,
                                          in_buffer_ptr->get_buffer(),
                                          in_offset,
                                          in_count_buffer_ptr->get_buffer(),
                                          in_count_offset,
                                          in_max_draw_count,
                                          in_stride);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_end_query(std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                Anvil::QueryIndex                 in_entry)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(EndQueryCommand(in_query_pool_ptr,
                                                 in_entry) );
        }
    }
    #endif

    vkCmdEndQuery(m_command_buffer,
                  in_query_pool_ptr->get_query_pool(),
                  in_entry);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_fill_buffer(std::shared_ptr<Anvil::Buffer> in_dst_buffer_ptr,
                                                  VkDeviceSize                   in_dst_offset,
                                                  VkDeviceSize                   in_size,
                                                  uint32_t                       in_data)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(FillBufferCommand(in_dst_buffer_ptr,
                                                   in_dst_offset,
                                                   in_size,
                                                   in_data) );
        }
    }
    #endif

    vkCmdFillBuffer(m_command_buffer,
                    in_dst_buffer_ptr->get_buffer(),
                    in_dst_offset,
                    in_size,
                    in_data);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_pipeline_barrier(VkPipelineStageFlags       in_src_stage_mask,
                                                       VkPipelineStageFlags       in_dst_stage_mask,
                                                       VkBool32                   in_by_region,
                                                       uint32_t                   in_memory_barrier_count,
                                                       const MemoryBarrier* const in_memory_barriers_ptr,
                                                       uint32_t                   in_buffer_memory_barrier_count,
                                                       const BufferBarrier* const in_buffer_memory_barriers_ptr,
                                                       uint32_t                   in_image_memory_barrier_count,
                                                       const ImageBarrier*  const in_image_memory_barriers_ptr)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    VkBufferMemoryBarrier buffer_barriers_vk[16];
    VkImageMemoryBarrier  image_barriers_vk [16];
    VkMemoryBarrier       memory_barriers_vk[16];
    bool                  result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(PipelineBarrierCommand(in_src_stage_mask,
                                                        in_dst_stage_mask,
                                                        in_by_region,
                                                        in_memory_barrier_count,
                                                        in_memory_barriers_ptr,
                                                        in_buffer_memory_barrier_count,
                                                        in_buffer_memory_barriers_ptr,
                                                        in_image_memory_barrier_count,
                                                        in_image_memory_barriers_ptr) );
        }
    }
    #endif

    if (get_n_of_callback_subscribers(COMMAND_BUFFER_CALLBACK_ID_PIPELINE_BARRIER_COMMAND_RECORDED) > 0)
    {
        PipelineBarrierCommand                     command_data(in_src_stage_mask,
                                                                in_dst_stage_mask,
                                                                in_by_region,
                                                                in_memory_barrier_count,
                                                                in_memory_barriers_ptr,
                                                                in_buffer_memory_barrier_count,
                                                                in_buffer_memory_barriers_ptr,
                                                                in_image_memory_barrier_count,
                                                                in_image_memory_barriers_ptr);
        PipelineBarrierCommandRecordedCallbackData callback_data(this,
                                                                &command_data);

        callback(COMMAND_BUFFER_CALLBACK_ID_PIPELINE_BARRIER_COMMAND_RECORDED,
                &callback_data);
    }

    anvil_assert(sizeof(buffer_barriers_vk) / sizeof(buffer_barriers_vk[0]) >= in_buffer_memory_barrier_count &&
                sizeof(image_barriers_vk)  / sizeof(image_barriers_vk [0]) >= in_image_memory_barrier_count  &&
                sizeof(memory_barriers_vk) / sizeof(memory_barriers_vk[0]) >= in_memory_barrier_count);

    for (uint32_t n_buffer_barrier = 0;
                  n_buffer_barrier < in_buffer_memory_barrier_count;
                ++n_buffer_barrier)
    {
        buffer_barriers_vk[n_buffer_barrier] = in_buffer_memory_barriers_ptr[n_buffer_barrier].get_barrier_vk();
    }

    for (uint32_t n_image_barrier = 0;
                  n_image_barrier < in_image_memory_barrier_count;
                ++n_image_barrier)
    {
        image_barriers_vk[n_image_barrier] = in_image_memory_barriers_ptr[n_image_barrier].get_barrier_vk();
    }

    for (uint32_t n_memory_barrier = 0;
                  n_memory_barrier < in_memory_barrier_count;
                ++n_memory_barrier)
    {
        memory_barriers_vk[n_memory_barrier] = in_memory_barriers_ptr[n_memory_barrier].get_barrier_vk();
    }

    vkCmdPipelineBarrier(m_command_buffer,
                         in_src_stage_mask,
                         in_dst_stage_mask,
                         in_by_region,
                         in_memory_barrier_count,
                         memory_barriers_vk,
                         in_buffer_memory_barrier_count,
                         buffer_barriers_vk,
                         in_image_memory_barrier_count,
                         image_barriers_vk);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_push_constants(std::shared_ptr<Anvil::PipelineLayout> in_layout_ptr,
                                                     VkShaderStageFlags                     in_stage_flags,
                                                     uint32_t                               in_offset,
                                                     uint32_t                               in_size,
                                                     const void*                            in_values)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(PushConstantsCommand(in_layout_ptr,
                                                      in_stage_flags,
                                                      in_offset,
                                                      in_size,
                                                      in_values) );
        }
    }
    #endif

    vkCmdPushConstants(m_command_buffer,
                       in_layout_ptr->get_pipeline_layout(),
                       in_stage_flags,
                       in_offset,
                       in_size,
                       in_values);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_reset_event(std::shared_ptr<Anvil::Event> in_event_ptr,
                                                  VkPipelineStageFlags          in_stage_mask)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ResetEventCommand(in_event_ptr,
                                                   in_stage_mask) );
        }
    }
    #endif

    vkCmdResetEvent(m_command_buffer,
                    in_event_ptr->get_event(),
                    in_stage_mask);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_reset_query_pool(std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                       Anvil::QueryIndex                 in_start_query,
                                                       uint32_t                          in_query_count)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ResetQueryPoolCommand(in_query_pool_ptr,
                                                       in_start_query,
                                                       in_query_count) );
        }
    }
    #endif

    vkCmdResetQueryPool(m_command_buffer,
                        in_query_pool_ptr->get_query_pool(),
                        in_start_query,
                        in_query_count);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_resolve_image(std::shared_ptr<Anvil::Image> in_src_image_ptr,
                                                    VkImageLayout                 in_src_image_layout,
                                                    std::shared_ptr<Anvil::Image> in_dst_image_ptr,
                                                    VkImageLayout                 in_dst_image_layout,
                                                    uint32_t                      in_region_count,
                                                    const VkImageResolve*         in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ResolveImageCommand(in_src_image_ptr,
                                                     in_src_image_layout,
                                                     in_dst_image_ptr,
                                                     in_dst_image_layout,
                                                     in_region_count,
                                                     in_region_ptrs) );
        }
    }
    #endif

    vkCmdResolveImage(m_command_buffer,
                      in_src_image_ptr->get_image(),
                      in_src_image_layout,
                      in_dst_image_ptr->get_image(),
                      in_dst_image_layout,
                      in_region_count,
                      in_region_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_blend_constants(const float in_blend_constants[4])
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetBlendConstantsCommand(in_blend_constants));
        }
    }
    #endif

    vkCmdSetBlendConstants(m_command_buffer,
                           in_blend_constants);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_depth_bias(float in_depth_bias_constant_factor,
                                                     float in_depth_bias_clamp,
                                                     float in_slope_scaled_depth_bias)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetDepthBiasCommand(in_depth_bias_constant_factor,
                                                     in_depth_bias_clamp,
                                                     in_slope_scaled_depth_bias) );
        }
    }
    #endif

    vkCmdSetDepthBias(m_command_buffer,
                      in_depth_bias_constant_factor,
                      in_depth_bias_clamp,
                      in_slope_scaled_depth_bias);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_depth_bounds(float in_min_depth_bounds,
                                                       float in_max_depth_bounds)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetDepthBoundsCommand(in_min_depth_bounds,
                                                       in_max_depth_bounds) );
        }
    }
    #endif

    vkCmdSetDepthBounds(m_command_buffer,
                        in_min_depth_bounds,
                        in_max_depth_bounds);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_event(std::shared_ptr<Anvil::Event> in_event_ptr,
                                                VkPipelineStageFlags          in_stage_mask)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetEventCommand(in_event_ptr,
                                                 in_stage_mask) );
        }
    }
    #endif

    vkCmdSetEvent(m_command_buffer,
                  in_event_ptr->get_event(),
                  in_stage_mask);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_line_width(float in_line_width)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetLineWidthCommand(in_line_width) );
        }
    }
    #endif

    vkCmdSetLineWidth(m_command_buffer,
                      in_line_width);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_scissor(uint32_t        in_first_scissor,
                                                  uint32_t        in_scissor_count,
                                                  const VkRect2D* in_scissor_ptrs)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetScissorCommand(in_first_scissor,
                                                   in_scissor_count,
                                                   in_scissor_ptrs) );
        }
    }
    #endif

    vkCmdSetScissor(m_command_buffer,
                    in_first_scissor,
                    in_scissor_count,
                    in_scissor_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_stencil_compare_mask(VkStencilFaceFlags in_face_mask,
                                                               uint32_t           in_stencil_compare_mask)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetStencilCompareMaskCommand(in_face_mask,
                                                              in_stencil_compare_mask) );
        }
    }
    #endif

    vkCmdSetStencilCompareMask(m_command_buffer,
                               in_face_mask,
                               in_stencil_compare_mask);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_stencil_reference(VkStencilFaceFlags in_face_mask,
                                                            uint32_t           in_stencil_reference)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetStencilReferenceCommand(in_face_mask,
                                                            in_stencil_reference) );
        }
    }
    #endif

    vkCmdSetStencilReference(m_command_buffer,
                             in_face_mask,
                             in_stencil_reference);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_stencil_write_mask(VkStencilFaceFlags in_face_mask,
                                                             uint32_t           in_stencil_write_mask)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetStencilWriteMaskCommand(in_face_mask,
                                                            in_stencil_write_mask) );
        }
    }
    #endif

    vkCmdSetStencilWriteMask(m_command_buffer,
                             in_face_mask,
                             in_stencil_write_mask);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_viewport(uint32_t          in_first_viewport,
                                                   uint32_t          in_viewport_count,
                                                   const VkViewport* in_viewport_ptrs)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetViewportCommand(in_first_viewport,
                                                    in_viewport_count,
                                                    in_viewport_ptrs) );
        }
    }
    #endif

    vkCmdSetViewport(m_command_buffer,
                     in_first_viewport,
                     in_viewport_count,
                     in_viewport_ptrs);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_update_buffer(std::shared_ptr<Anvil::Buffer> in_dst_buffer_ptr,
                                                    VkDeviceSize                   in_dst_offset,
                                                    VkDeviceSize                   in_data_size,
                                                    const uint32_t*                in_data_ptr)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(UpdateBufferCommand(in_dst_buffer_ptr,
                                                     in_dst_offset,
                                                     in_data_size,
                                                     in_data_ptr) );
        }
    }
    #endif

    vkCmdUpdateBuffer(m_command_buffer,
                      in_dst_buffer_ptr->get_buffer(),
                      in_dst_offset,
                      in_data_size,
                      in_data_ptr);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_wait_events(uint32_t                       in_event_count,
                                                  std::shared_ptr<Anvil::Event>* in_events,
                                                  VkPipelineStageFlags           in_src_stage_mask,
                                                  VkPipelineStageFlags           in_dst_stage_mask,
                                                  uint32_t                       in_memory_barrier_count,
                                                  const MemoryBarrier* const     in_memory_barriers_ptr,
                                                  uint32_t                       in_buffer_memory_barrier_count,
                                                  const BufferBarrier* const     in_buffer_memory_barriers_ptr,
                                                  uint32_t                       in_image_memory_barrier_count,
                                                  const ImageBarrier* const      in_image_memory_barriers_ptr)

{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    VkEvent               events            [16];
    VkBufferMemoryBarrier buffer_barriers_vk[16];
    VkImageMemoryBarrier  image_barriers_vk [16];
    VkMemoryBarrier       memory_barriers_vk[16];
    bool                  result = false;

    anvil_assert(in_event_count                 > 0); /* as per spec - easy to miss */
    anvil_assert(in_event_count                 < sizeof(events)             / sizeof(events            [0]) );
    anvil_assert(in_buffer_memory_barrier_count < sizeof(buffer_barriers_vk) / sizeof(buffer_barriers_vk[0]) );
    anvil_assert(in_image_memory_barrier_count  < sizeof(image_barriers_vk)  / sizeof(image_barriers_vk [0]) );
    anvil_assert(in_memory_barrier_count        < sizeof(memory_barriers_vk) / sizeof(memory_barriers_vk[0]) );

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(WaitEventsCommand(in_event_count,
                                                   in_events,
                                                   in_src_stage_mask,
                                                   in_dst_stage_mask,
                                                   in_memory_barrier_count,
                                                   in_memory_barriers_ptr,
                                                   in_buffer_memory_barrier_count,
                                                   in_buffer_memory_barriers_ptr,
                                                   in_image_memory_barrier_count,
                                                   in_image_memory_barriers_ptr) );
        }
    }
    #endif

    for (uint32_t n_event = 0;
                  n_event < in_event_count;
                ++n_event)
    {
        events[n_event] = in_events[n_event]->get_event();
    }

    for (uint32_t n_buffer_barrier = 0;
                  n_buffer_barrier < in_buffer_memory_barrier_count;
                ++n_buffer_barrier)
    {
        buffer_barriers_vk[n_buffer_barrier] = in_buffer_memory_barriers_ptr[n_buffer_barrier].get_barrier_vk();
    }

    for (uint32_t n_image_barrier = 0;
                  n_image_barrier < in_image_memory_barrier_count;
                ++n_image_barrier)
    {
        image_barriers_vk[n_image_barrier] = in_image_memory_barriers_ptr[n_image_barrier].get_barrier_vk();
    }

    for (uint32_t n_memory_barrier = 0;
                  n_memory_barrier < in_memory_barrier_count;
                ++n_memory_barrier)
    {
        memory_barriers_vk[n_memory_barrier] = in_memory_barriers_ptr[n_memory_barrier].get_barrier_vk();
    }

    vkCmdWaitEvents(m_command_buffer,
                    in_event_count,
                    events,
                    in_src_stage_mask,
                    in_dst_stage_mask,
                    in_memory_barrier_count,
                    memory_barriers_vk,
                    in_buffer_memory_barrier_count,
                    buffer_barriers_vk,
                    in_image_memory_barrier_count,
                    image_barriers_vk);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_write_timestamp(VkPipelineStageFlagBits           in_pipeline_stage,
                                                      std::shared_ptr<Anvil::QueryPool> in_query_pool_ptr,
                                                      Anvil::QueryIndex                 in_query_index)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(WriteTimestampCommand(in_pipeline_stage,
                                                       in_query_pool_ptr,
                                                       in_query_index) );
        }
    }
    #endif

    vkCmdWriteTimestamp(m_command_buffer,
                        in_pipeline_stage,
                        in_query_pool_ptr->get_query_pool(),
                        in_query_index);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::reset(bool should_release_resources)
{
    bool     result    = false;
    VkResult result_vk;

    if (m_recording_in_progress)
    {
        anvil_assert(!m_recording_in_progress);

        goto end;
    }

    result_vk = vkResetCommandBuffer(m_command_buffer,
                                     (should_release_resources) ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);

    if (!is_vk_call_successful(result_vk) )
    {
        anvil_assert_vk_call_succeeded(result_vk);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        clear_commands();
    }
    #endif

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::stop_recording()
{
    bool     result = false;
    VkResult result_vk;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    result_vk = vkEndCommandBuffer(m_command_buffer);

    if (!is_vk_call_successful(result_vk))
    {
        anvil_assert_vk_call_succeeded(result_vk);

        goto end;
    }

    m_recording_in_progress = false;
    result                  = true;
end:
    return result;
}

/* Please see header for specification */
Anvil::PrimaryCommandBuffer::PrimaryCommandBuffer(std::weak_ptr<Anvil::Device>        device_ptr,
                                                  std::shared_ptr<Anvil::CommandPool> parent_command_pool_ptr)
    :CommandBufferBase(device_ptr,
                       parent_command_pool_ptr,
                       COMMAND_BUFFER_TYPE_PRIMARY)
{
    VkCommandBufferAllocateInfo    alloc_info;
    std::shared_ptr<Anvil::Device> device_locked_ptr(device_ptr);
    VkResult                       result_vk;

    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool        = parent_command_pool_ptr->get_command_pool();
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.pNext              = nullptr;
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    result_vk = vkAllocateCommandBuffers(device_locked_ptr->get_device_vk(),
                                        &alloc_info,
                                        &m_command_buffer);

    anvil_assert_vk_call_succeeded(result_vk);
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::record_begin_render_pass(uint32_t                            in_n_clear_values,
                                                           const VkClearValue*                 in_clear_value_ptrs,
                                                           std::shared_ptr<Anvil::Framebuffer> in_fbo_ptr,
                                                           VkRect2D                            in_render_area,
                                                           std::shared_ptr<Anvil::RenderPass>  in_render_pass_ptr,
                                                           VkSubpassContents                   in_contents)
{
    VkRenderPassBeginInfo render_pass_begin_info;
    bool                  result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    if (get_n_of_callback_subscribers(COMMAND_BUFFER_CALLBACK_ID_BEGIN_RENDER_PASS_COMMAND_RECORDED) > 0)
    {
        BeginRenderPassCommand                     command_data(in_n_clear_values,
                                                                in_clear_value_ptrs,
                                                                in_fbo_ptr,
                                                                in_render_area,
                                                                in_render_pass_ptr,
                                                                in_contents);
        BeginRenderPassCommandRecordedCallbackData callback_data(this,
                                                                &command_data);

        callback(COMMAND_BUFFER_CALLBACK_ID_BEGIN_RENDER_PASS_COMMAND_RECORDED,
                &callback_data);
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BeginRenderPassCommand(in_n_clear_values,
                                                        in_clear_value_ptrs,
                                                        in_fbo_ptr,
                                                        in_render_area,
                                                        in_render_pass_ptr,
                                                        in_contents) );
        }
    }
    #endif

    render_pass_begin_info.clearValueCount = in_n_clear_values;
    render_pass_begin_info.framebuffer     = in_fbo_ptr->get_framebuffer(in_render_pass_ptr);
    render_pass_begin_info.pClearValues    = in_clear_value_ptrs;
    render_pass_begin_info.pNext           = nullptr;
    render_pass_begin_info.renderArea      = in_render_area;
    render_pass_begin_info.renderPass      = in_render_pass_ptr->get_render_pass();
    render_pass_begin_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    vkCmdBeginRenderPass(m_command_buffer,
                        &render_pass_begin_info,
                         in_contents);

    m_is_renderpass_active = true;
    result                 = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::record_end_render_pass()
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    if (get_n_of_callback_subscribers(COMMAND_BUFFER_CALLBACK_ID_END_RENDER_PASS_COMMAND_RECORDED) > 0)
    {
        EndRenderPassCommand                     command_data;
        EndRenderPassCommandRecordedCallbackData callback_data(this,
                                                              &command_data);

        callback(COMMAND_BUFFER_CALLBACK_ID_END_RENDER_PASS_COMMAND_RECORDED,
                &callback_data);
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(EndRenderPassCommand() );
        }
    }
    #endif

    vkCmdEndRenderPass(m_command_buffer);

    m_is_renderpass_active = false;
    result                 = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::record_execute_commands(uint32_t                                        in_cmd_buffers_count,
                                                          std::shared_ptr<Anvil::SecondaryCommandBuffer>* in_cmd_buffer_ptrs)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    VkCommandBuffer cmd_buffers[16];
    bool            result = false;

    anvil_assert(in_cmd_buffers_count < sizeof(cmd_buffers) / sizeof(cmd_buffers[0]) );

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ExecuteCommandsCommand(in_cmd_buffers_count,
                                                        in_cmd_buffer_ptrs) );
        }
    }
    #endif

    for (uint32_t n_cmd_buffer = 0;
                  n_cmd_buffer < in_cmd_buffers_count;
                ++n_cmd_buffer)
    {
        cmd_buffers[n_cmd_buffer] = in_cmd_buffer_ptrs[n_cmd_buffer]->get_command_buffer();
    }

    vkCmdExecuteCommands(m_command_buffer,
                         in_cmd_buffers_count,
                         cmd_buffers);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::record_next_subpass(VkSubpassContents in_contents)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(NextSubpassCommand(in_contents) );
        }
    }
    #endif

    vkCmdNextSubpass(m_command_buffer,
                     in_contents);

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::start_recording(bool one_time_submit,
                                                  bool simultaneous_use_allowed)
{
    VkCommandBufferBeginInfo command_buffer_begin_info;
    bool                     result    = false;
    VkResult                 result_vk;

    if (m_recording_in_progress)
    {
        anvil_assert(!m_recording_in_progress);

        goto end;
    }

    command_buffer_begin_info.flags            = ((one_time_submit)          ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT  : 0) |
                                                 ((simultaneous_use_allowed) ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0);
    command_buffer_begin_info.pNext            = nullptr;
    command_buffer_begin_info.pInheritanceInfo = nullptr;  /* Only relevant for secondary-level command buffers */
    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    result_vk = vkBeginCommandBuffer(m_command_buffer,
                                    &command_buffer_begin_info);

    if (!is_vk_call_successful(result_vk) )
    {
        anvil_assert_vk_call_succeeded(result_vk);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        /* vkBeginCommandBuffer() implicitly resets all commands recorded previously */
        clear_commands();
    }
    #endif

    m_recording_in_progress = true;
    result                  = true;

end:
    return result;
}


/* Please see header for specification */
Anvil::SecondaryCommandBuffer::SecondaryCommandBuffer(std::weak_ptr<Anvil::Device>        device_ptr,
                                                      std::shared_ptr<Anvil::CommandPool> parent_command_pool_ptr)
    :CommandBufferBase(device_ptr,
                       parent_command_pool_ptr,
                       COMMAND_BUFFER_TYPE_SECONDARY)
{
    VkCommandBufferAllocateInfo    command_buffer_alloc_info;
    std::shared_ptr<Anvil::Device> device_locked_ptr(device_ptr);
    VkResult                       result_vk;

    command_buffer_alloc_info.commandBufferCount = 1;
    command_buffer_alloc_info.commandPool        = parent_command_pool_ptr->get_command_pool();
    command_buffer_alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_alloc_info.pNext              = nullptr;
    command_buffer_alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    result_vk = vkAllocateCommandBuffers(device_locked_ptr->get_device_vk(),
                                        &command_buffer_alloc_info,
                                        &m_command_buffer);

    anvil_assert_vk_call_succeeded(result_vk);
}

/* Please see header for specification */
bool Anvil::SecondaryCommandBuffer::start_recording(bool                          one_time_submit,
                                                    bool                          simultaneous_use_allowed,
                                                    bool                          renderpass_usage_only,
                                                    std::shared_ptr<Framebuffer>  framebuffer_ptr,
                                                    std::shared_ptr<RenderPass>   render_pass_ptr,
                                                    SubPassID                     subpass_id,
                                                    OcclusionQuerySupportScope    required_occlusion_query_support_scope,
                                                    VkQueryPipelineStatisticFlags required_pipeline_statistics_scope)
{
    VkCommandBufferBeginInfo       command_buffer_begin_info;
    VkCommandBufferInheritanceInfo command_buffer_inheritance_info;
    bool                           result    = false;
    VkResult                       result_vk;

    if (m_recording_in_progress)
    {
        anvil_assert(!m_recording_in_progress);

        goto end;
    }

    command_buffer_inheritance_info.framebuffer          = framebuffer_ptr->get_framebuffer(render_pass_ptr);
    command_buffer_inheritance_info.occlusionQueryEnable = (required_occlusion_query_support_scope != OCCLUSION_QUERY_SUPPORT_SCOPE_NOT_REQUIRED);
    command_buffer_inheritance_info.pipelineStatistics   = required_pipeline_statistics_scope;
    command_buffer_inheritance_info.pNext                = nullptr;
    command_buffer_inheritance_info.queryFlags           = (required_occlusion_query_support_scope == OCCLUSION_QUERY_SUPPORT_SCOPE_REQUIRED);
    command_buffer_inheritance_info.renderPass           = render_pass_ptr->get_render_pass();
    command_buffer_inheritance_info.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    command_buffer_inheritance_info.subpass              = subpass_id;

    command_buffer_begin_info.flags            = ((one_time_submit)          ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT      : 0) |
                                                 ((simultaneous_use_allowed) ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT     : 0) |
                                                 ((renderpass_usage_only)    ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0);
    command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;
    command_buffer_begin_info.pNext            = nullptr;
    command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    result_vk = vkBeginCommandBuffer(m_command_buffer,
                                    &command_buffer_begin_info);

    if (!is_vk_call_successful(result_vk) )
    {
        anvil_assert_vk_call_succeeded(result_vk);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        /* vkBeginCommandBuffer() implicitly resets all commands recorded previously */
        clear_commands();
    }
    #endif

    m_is_renderpass_active = renderpass_usage_only;

    m_recording_in_progress = true;
    result                  = true;

end:
    return result;
}
