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

#include "misc/debug.h"
#include "misc/object_tracker.h"
#include "wrappers/buffer.h"
#include "wrappers/command_buffer.h"
#include "wrappers/command_pool.h"
#include "wrappers/device.h"
#include "wrappers/memory_block.h"
#include "wrappers/queue.h"

/* Please see header for specification */
Anvil::Buffer::Buffer(std::weak_ptr<Anvil::Device> device_ptr,
                      VkDeviceSize                 size,
                      QueueFamilyBits              queue_families,
                      VkSharingMode                queue_sharing_mode,
                      VkBufferUsageFlags           usage_flags)
    :m_buffer           (VK_NULL_HANDLE),
     m_buffer_size      (size),
     m_device_ptr       (device_ptr),
     m_memory_block_ptr (nullptr),
     m_parent_buffer_ptr(0),
     m_start_offset     (0)
{
    /* Assume the user may try to bind memory from non-mappable memory heap, in which case
     * we're going to need to copy data from a staging buffer to this buffer if the user
     * ever uses write(), and vice versa. */
    m_usage_flags = static_cast<VkBufferUsageFlagBits>(usage_flags                       |
                                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT  |
                                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
}

/* Please see header for specification */
Anvil::Buffer::Buffer(std::weak_ptr<Anvil::Device> device_ptr,
                      VkDeviceSize                 size,
                      Anvil::QueueFamilyBits       queue_families,
                      VkSharingMode                queue_sharing_mode,
                      VkBufferUsageFlags           usage_flags,
                      bool                         should_be_mappable,
                      bool                         should_be_coherent,
                      const void*                  opt_client_data)
    :m_buffer           (VK_NULL_HANDLE),
     m_buffer_size      (size),
     m_device_ptr       (device_ptr),
     m_memory_block_ptr (nullptr),
     m_parent_buffer_ptr(0),
     m_start_offset     (0),
     m_usage_flags      (static_cast<VkBufferUsageFlagBits>(usage_flags) )
{
    /* Sanity checks */
    if (!should_be_mappable)
    {
        anvil_assert(!should_be_coherent);

        /* For host->gpu writes to work in this case, we will need the buffer to work as a target
         * for buffer->buffer copy operations. Same goes for the other way around.
         */
        m_usage_flags = static_cast<VkBufferUsageFlagBits>(m_usage_flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT 
                                                                         | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    }
}

/* Please see header for specification */
Anvil::Buffer::Buffer(std::shared_ptr<Anvil::Buffer> parent_buffer_ptr,
                      VkDeviceSize                   start_offset,
                      VkDeviceSize                   size)
    :m_buffer           (VK_NULL_HANDLE),
     m_buffer_size      (size),
     m_memory_block_ptr (nullptr),
     m_parent_buffer_ptr(parent_buffer_ptr),
     m_start_offset     (start_offset)
{
    /* Sanity checks */
    anvil_assert(parent_buffer_ptr != nullptr);
    anvil_assert(start_offset      >= 0);
    anvil_assert(size              >  0);

    m_usage_flags = parent_buffer_ptr->m_usage_flags;
}


/** Releases a buffer object and a memory object associated with this Buffer instance. */
Anvil::Buffer::~Buffer()
{
    if (m_buffer            != VK_NULL_HANDLE &&
        m_parent_buffer_ptr == nullptr)
    {
        std::shared_ptr<Device> device_locked_ptr(m_device_ptr);

        vkDestroyBuffer(device_locked_ptr->get_device_vk(),
                        m_buffer,
                        nullptr /* pAllocator */);

        m_buffer = VK_NULL_HANDLE;
    }

    /* Unregister the object */
    Anvil::ObjectTracker::get()->unregister_object(Anvil::ObjectTracker::OBJECT_TYPE_BUFFER,
                                                   this);
}

/** Please see header for specification */
std::shared_ptr<Anvil::Buffer> Anvil::Buffer::create(std::weak_ptr<Anvil::Device> device_ptr,
                                                     VkDeviceSize                 size,
                                                     QueueFamilyBits              queue_families,
                                                     VkSharingMode                queue_sharing_mode,
                                                     VkBufferUsageFlags           usage_flags)
{
    std::shared_ptr<Anvil::Buffer> new_buffer_ptr;

    new_buffer_ptr.reset(
        new Anvil::Buffer(device_ptr,
                          size,
                          queue_families,
                          queue_sharing_mode,
                          usage_flags)
    );

    /* Initialize */
    new_buffer_ptr->create_buffer(queue_families,
                                  queue_sharing_mode,
                                  size);

    /* Register the object */
    Anvil::ObjectTracker::get()->register_object(Anvil::ObjectTracker::OBJECT_TYPE_BUFFER,
                                                 new_buffer_ptr.get() );

    return new_buffer_ptr;
}

/** Please see header for specification */
std::shared_ptr<Anvil::Buffer> Anvil::Buffer::create(std::weak_ptr<Anvil::Device> device_ptr,
                                                     VkDeviceSize                 size,
                                                     QueueFamilyBits              queue_families,
                                                     VkSharingMode                queue_sharing_mode,
                                                     VkBufferUsageFlags           usage_flags,
                                                     bool                         should_be_mappable,
                                                     bool                         should_be_coherent,
                                                     const void*                  opt_client_data)
{
    std::shared_ptr<Anvil::Buffer> new_buffer_ptr;

    new_buffer_ptr.reset(
        new Anvil::Buffer(device_ptr,
                          size,
                          queue_families,
                          queue_sharing_mode,
                          usage_flags,
                          should_be_mappable,
                          should_be_coherent,
                          opt_client_data)
    );

    /* Initialize */
    new_buffer_ptr->create_buffer(queue_families,
                                  queue_sharing_mode,
                                  size);

    /* Create a memory object and preallocate as much space as we need */
    {
        std::shared_ptr<Anvil::MemoryBlock> memory_block_ptr;

        memory_block_ptr = Anvil::MemoryBlock::create(device_ptr,
                                                      new_buffer_ptr->m_buffer_memory_reqs.memoryTypeBits,
                                                      new_buffer_ptr->m_buffer_memory_reqs.size,
                                                      should_be_mappable,
                                                      should_be_coherent);

        new_buffer_ptr->set_memory(memory_block_ptr);

        if (opt_client_data != nullptr)
        {
            new_buffer_ptr->write(0,
                                  size,
                                  opt_client_data);
        }
    }

    /* Register the object */
    Anvil::ObjectTracker::get()->register_object(Anvil::ObjectTracker::OBJECT_TYPE_BUFFER,
                                                 new_buffer_ptr.get() );

    return new_buffer_ptr;
}

/** Please see header for specification */
std::shared_ptr<Anvil::Buffer> Anvil::Buffer::create(std::shared_ptr<Anvil::Buffer> parent_buffer_ptr,
                                                     VkDeviceSize                   start_offset,
                                                     VkDeviceSize                   size)
{
    std::shared_ptr<Anvil::Buffer> new_buffer_ptr;

    new_buffer_ptr.reset(
        new Anvil::Buffer(parent_buffer_ptr,
                          start_offset,
                          size)
    );

    /* Initialize */
    new_buffer_ptr->m_memory_block_ptr = Anvil::MemoryBlock::create_derived(parent_buffer_ptr->m_memory_block_ptr,
                                                                            start_offset,
                                                                            size);

    new_buffer_ptr->m_buffer = parent_buffer_ptr->m_buffer;

    /* Register the object */
    Anvil::ObjectTracker::get()->register_object(Anvil::ObjectTracker::OBJECT_TYPE_BUFFER,
                                                 new_buffer_ptr.get() );

    return new_buffer_ptr;
}

/** Converts a Anvil::QueueFamilyBits bitfield value to an array of queue family indices.
 *
 *  @param queue_families                     Input value to convert from.
 *  @param out_opt_queue_family_indices_ptr   If not NULL, deref will be updated with @param *out_opt_n_queue_family_indices_ptr
 *                                            values, corresponding to queue family indices, as specified under @param queue_families.
 *  @param out_opt_n_queue_family_indices_ptr If not NULL, deref will be set to the number of items that would be or were written
 *                                            under @param out_opt_queue_family_indices_ptr.
 *
 **/
void Anvil::Buffer::convert_queue_family_bits_to_family_indices(Anvil::QueueFamilyBits queue_families,
                                                                uint32_t*              out_opt_queue_family_indices_ptr,
                                                                uint32_t*              out_opt_n_queue_family_indices_ptr) const
{
    std::shared_ptr<Device> device_locked_ptr(m_device_ptr);
    uint32_t                n_queue_family_indices = 0;

    if ((queue_families & QUEUE_FAMILY_COMPUTE_BIT) != 0)
    {
        anvil_assert(device_locked_ptr->get_n_compute_queues() > 0);

        if (out_opt_queue_family_indices_ptr != nullptr)
        {
            out_opt_queue_family_indices_ptr[n_queue_family_indices] = device_locked_ptr->get_queue_family_index(Anvil::QUEUE_FAMILY_TYPE_COMPUTE);
        }

        ++n_queue_family_indices;
    }

    if ((queue_families & QUEUE_FAMILY_DMA_BIT) != 0)
    {
        anvil_assert(device_locked_ptr->get_n_transfer_queues() > 0);

        if (out_opt_queue_family_indices_ptr != nullptr)
        {
            out_opt_queue_family_indices_ptr[n_queue_family_indices] = device_locked_ptr->get_queue_family_index(Anvil::QUEUE_FAMILY_TYPE_TRANSFER);
        }

        ++n_queue_family_indices;
    }

    if ((queue_families & QUEUE_FAMILY_GRAPHICS_BIT) != 0)
    {
        anvil_assert(device_locked_ptr->get_n_universal_queues() > 0);

        if (out_opt_queue_family_indices_ptr != nullptr)
        {
            out_opt_queue_family_indices_ptr[n_queue_family_indices] = device_locked_ptr->get_queue_family_index(Anvil::QUEUE_FAMILY_TYPE_UNIVERSAL);
        }

        ++n_queue_family_indices;
    }

    if (out_opt_n_queue_family_indices_ptr != nullptr)
    {
        *out_opt_n_queue_family_indices_ptr = n_queue_family_indices;
    }
}

/* Creates a new Vulkan buffer object and caches memory requirements for the created buffer.
 *
 * @param queue_families Queue families the buffer needs to support.
 * @param sharing_mode   Sharing mode the buffer needs to support.
 * @param size           Size of the buffer.
 **/
void Anvil::Buffer::create_buffer(Anvil::QueueFamilyBits queue_families,
                                  VkSharingMode          sharing_mode,
                                  VkDeviceSize           size)
{
    VkBufferCreateInfo      buffer_create_info;
    std::shared_ptr<Device> device_locked_ptr(m_device_ptr);
    uint32_t                n_queue_family_indices;
    uint32_t                queue_family_indices[8];
    VkResult                result;

    /* Determine which queues the buffer should be available to. */
    convert_queue_family_bits_to_family_indices(queue_families,
                                                queue_family_indices,
                                               &n_queue_family_indices);

    anvil_assert(n_queue_family_indices > 0);
    anvil_assert(n_queue_family_indices < sizeof(queue_family_indices) / sizeof(queue_family_indices[0]) );

    /* Prepare the create info structure */
    buffer_create_info.flags                 = 0;
    buffer_create_info.pNext                 = nullptr;
    buffer_create_info.pQueueFamilyIndices   = queue_family_indices;
    buffer_create_info.queueFamilyIndexCount = n_queue_family_indices;
    buffer_create_info.sharingMode           = sharing_mode;
    buffer_create_info.size                  = size;
    buffer_create_info.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage                 = m_usage_flags;

    /* Create the buffer object */
    result = vkCreateBuffer(device_locked_ptr->get_device_vk(),
                           &buffer_create_info,
                            nullptr, /* pAllocator */
                           &m_buffer);
    anvil_assert_vk_call_succeeded(result);

    /* Cache buffer data memory requirements */
    vkGetBufferMemoryRequirements(device_locked_ptr->get_device_vk(),
                                  m_buffer,
                                 &m_buffer_memory_reqs);
}

/* Please see header for specification */
std::shared_ptr<Anvil::Buffer> Anvil::Buffer::get_base_buffer()
{
    std::shared_ptr<Anvil::Buffer> parent_ptr;
    std::shared_ptr<Anvil::Buffer> result_ptr = shared_from_this();

    if ( (parent_ptr = result_ptr->get_parent_buffer_ptr()) != nullptr)
    {
        result_ptr = parent_ptr;
    }

    return result_ptr;
}

/* Please see header for specification */
VkDeviceSize Anvil::Buffer::get_size() const
{
    anvil_assert(m_buffer_size != 0);

    return m_buffer_size;
}

/* Please see header for specification */
VkDeviceSize Anvil::Buffer::get_start_offset() const
{
    if (m_memory_block_ptr != nullptr)
    {
        return m_start_offset;
    }
    else
    {
        anvil_assert(false);

        return -1;
    }
}

/* Please see header for specification */
bool Anvil::Buffer::read(VkDeviceSize start_offset,
                         VkDeviceSize size,
                         void*        out_result_ptr)
{
    std::shared_ptr<Device> device_locked_ptr(m_device_ptr);
    bool                    result = false;

    anvil_assert(m_memory_block_ptr != nullptr);

    if (m_memory_block_ptr->is_mappable() )
    {
        result = m_memory_block_ptr->read(start_offset,
                                          size,
                                          out_result_ptr);
    }
    else
    {
        /* The buffer memory is not mappable. We need to create a staging buffer,
         * do a non-mappable->mappable memory copy, and then read back data from the mappable buffer. */
        const uint32_t                 n_transfer_queues             = device_locked_ptr->get_n_transfer_queues();
        std::shared_ptr<Anvil::Queue>  queue_ptr                     = (n_transfer_queues > 0) ? device_locked_ptr->get_transfer_queue (0)
                                                                                               : device_locked_ptr->get_universal_queue(0);
        std::shared_ptr<Anvil::Buffer> staging_buffer_ptr            = nullptr;
        const Anvil::QueueFamilyBits   staging_buffer_queue_fam_bits = (n_transfer_queues > 0) ? Anvil::QUEUE_FAMILY_DMA_BIT       : Anvil::QUEUE_FAMILY_GRAPHICS_BIT;
        const Anvil::QueueFamilyType   staging_buffer_queue_fam_type = (n_transfer_queues > 0) ? Anvil::QUEUE_FAMILY_TYPE_TRANSFER : Anvil::QUEUE_FAMILY_TYPE_UNIVERSAL;

        std::shared_ptr<Anvil::PrimaryCommandBuffer> copy_cmdbuf_ptr = device_locked_ptr->get_command_pool(staging_buffer_queue_fam_type)->alloc_primary_level_command_buffer();

        if (copy_cmdbuf_ptr == nullptr)
        {
            anvil_assert(copy_cmdbuf_ptr != nullptr);

            goto end;
        }

        staging_buffer_ptr = Anvil::Buffer::create(
            m_device_ptr,
            size,
            staging_buffer_queue_fam_bits,
            VK_SHARING_MODE_EXCLUSIVE,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            true,  /* should_be_mappable */
            false, /* should_be_coherent */
            nullptr);

        if (staging_buffer_ptr == nullptr)
        {
            anvil_assert(staging_buffer_ptr != nullptr);

            goto end;
        }

        copy_cmdbuf_ptr->start_recording(true,   /* one_time_submit          */
                                         false); /* simultaneous_use_allowed */
        {
            VkBufferCopy copy_region;

            copy_region.dstOffset = 0;
            copy_region.size      = size;
            copy_region.srcOffset = start_offset;

            copy_cmdbuf_ptr->record_copy_buffer(shared_from_this(),
                                                staging_buffer_ptr,
                                                1, /* in_region_count */
                                               &copy_region);
        }
        copy_cmdbuf_ptr->stop_recording();

        queue_ptr->submit_command_buffer(copy_cmdbuf_ptr,
                                         true /* should_block */);

        result = staging_buffer_ptr->read(start_offset,
                                          size,
                                          out_result_ptr);
    }

end:
    return result;
}

/* Please see header for specification */
bool Anvil::Buffer::set_memory(std::shared_ptr<Anvil::MemoryBlock> memory_block_ptr)
{
    std::shared_ptr<Device> device_locked_ptr(m_device_ptr);
    bool                    result = false;
    VkResult                result_vk;

    if (memory_block_ptr == nullptr)
    {
        anvil_assert(!(memory_block_ptr == nullptr) );

        goto end;
    }

    if (m_memory_block_ptr != nullptr)
    {
        anvil_assert( (memory_block_ptr == nullptr) );

        goto end;
    }

    /* Bind the memory object to the buffer object */
    m_memory_block_ptr = memory_block_ptr;

    result_vk = vkBindBufferMemory(device_locked_ptr->get_device_vk(),
                                   m_buffer,
                                   m_memory_block_ptr->get_memory(),
                                   memory_block_ptr->get_start_offset() );
    anvil_assert_vk_call_succeeded(result_vk);

    result = is_vk_call_successful(result_vk);
end:
    return result;
}

/* Please see header for specification */
bool Anvil::Buffer::write(VkDeviceSize start_offset,
                          VkDeviceSize size,
                          const void*  data)
{
    std::shared_ptr<Device> device_locked_ptr(m_device_ptr);
    bool                    result = false;

    anvil_assert(m_memory_block_ptr != nullptr);

    if (m_memory_block_ptr->is_mappable() )
    {
        result = m_memory_block_ptr->write(start_offset,
                                           size,
                                           data);
    }
    else
    {
        /* The buffer memory is not mappable. We need to create a staging memory,
         * upload user's data there, and then issue a copy op. */
        const uint32_t                 n_transfer_queues             = device_locked_ptr->get_n_transfer_queues();
        std::shared_ptr<Anvil::Queue>  queue_ptr                     = (n_transfer_queues > 0) ? device_locked_ptr->get_transfer_queue (0)
                                                                                               : device_locked_ptr->get_universal_queue(0);
        std::shared_ptr<Anvil::Buffer> staging_buffer_ptr            = nullptr;
        const Anvil::QueueFamilyBits   staging_buffer_queue_fam_bits = (n_transfer_queues > 0) ? Anvil::QUEUE_FAMILY_DMA_BIT       : Anvil::QUEUE_FAMILY_GRAPHICS_BIT;
        const Anvil::QueueFamilyType   staging_buffer_queue_fam_type = (n_transfer_queues > 0) ? Anvil::QUEUE_FAMILY_TYPE_TRANSFER : Anvil::QUEUE_FAMILY_TYPE_UNIVERSAL;

        std::shared_ptr<Anvil::PrimaryCommandBuffer> copy_cmdbuf_ptr = device_locked_ptr->get_command_pool(staging_buffer_queue_fam_type)->alloc_primary_level_command_buffer();

        if (copy_cmdbuf_ptr == nullptr)
        {
            anvil_assert(copy_cmdbuf_ptr != nullptr);

            goto end;
        }

        staging_buffer_ptr = Anvil::Buffer::create(
            m_device_ptr,
            size,
            staging_buffer_queue_fam_bits,
            VK_SHARING_MODE_EXCLUSIVE,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            true,  /* should_be_mappable */
            false, /* should_be_coherent */
            data);

        if (staging_buffer_ptr == nullptr)
        {
            anvil_assert(staging_buffer_ptr != nullptr);

            goto end;
        }

        copy_cmdbuf_ptr->start_recording(true,   /* one_time_submit          */
                                         false); /* simultaneous_use_allowed */
        {
            VkBufferCopy copy_region;

            copy_region.dstOffset = start_offset;
            copy_region.size      = size;
            copy_region.srcOffset = 0;

            copy_cmdbuf_ptr->record_copy_buffer(staging_buffer_ptr,
                                                shared_from_this(),
                                                1, /* in_region_count */
                                               &copy_region);
        }
        copy_cmdbuf_ptr->stop_recording();

        queue_ptr->submit_command_buffer(copy_cmdbuf_ptr,
                                         true /* should_block */);

        result = true;
    }

end:
    return result;
}