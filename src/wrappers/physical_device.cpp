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
#include "wrappers/instance.h"
#include "wrappers/physical_device.h"
#include <algorithm>

/* Please see header for specification */
std::weak_ptr<Anvil::PhysicalDevice> Anvil::PhysicalDevice::create(std::shared_ptr<Anvil::Instance> instance_ptr,
                                                                   uint32_t                         index,
                                                                   VkPhysicalDevice                 physical_device)
{
    std::shared_ptr<Anvil::PhysicalDevice> physical_device_ptr(new Anvil::PhysicalDevice(instance_ptr,
                                                                                         index,
                                                                                         physical_device) );

    physical_device_ptr->init             ();
    instance_ptr->register_physical_device(physical_device_ptr);

    /* Register the physical device instance */
    Anvil::ObjectTracker::get()->register_object(Anvil::ObjectTracker::OBJECT_TYPE_PHYSICAL_DEVICE,
                                                 physical_device_ptr.get() );

    return physical_device_ptr;
}

/* Please see header for specification */
Anvil::PhysicalDevice::~PhysicalDevice()
{
    anvil_assert(m_destroyed);

    Anvil::ObjectTracker::get()->unregister_object(Anvil::ObjectTracker::OBJECT_TYPE_PHYSICAL_DEVICE,
                                                    this);
}

/* Please see header for specification */
void Anvil::PhysicalDevice::destroy()
{
    anvil_assert(!m_destroyed);

    m_destroyed = true;

    while (m_cached_devices.size() > 0)
    {
        m_cached_devices.back()->destroy();
    }

    m_instance_ptr->unregister_physical_device(shared_from_this() );

}

/** Initializes _vulkan_physical_device descriptor with actual data by issuing a number
 *  of Vulkan API calls.
 *
 *  @param physical_device_ptr Pointer to a _vulkan_physical_device instance to be initialized.
 *                             Must not be nullptr.
 **/
void Anvil::PhysicalDevice::init()
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    uint32_t                         n_extensions             = 0;
    uint32_t                         n_physical_device_layers = 0;
    uint32_t                         n_physical_device_queues = 0;
    VkResult                         result;

    anvil_assert(m_physical_device != VK_NULL_HANDLE);

    /* Retrieve device features */
    vkGetPhysicalDeviceFeatures(m_physical_device,
                               &m_features);

    /* Retrieve device format properties */
    for (VkFormat current_format = (VkFormat) 1; /* skip the _UNDEFINED format */
                  current_format < VK_FORMAT_END_RANGE;
          ++(int&)current_format)
    {
        VkFormatProperties format_props;

        vkGetPhysicalDeviceFormatProperties(m_physical_device,
                                            current_format,
                                           &format_props);

        m_format_properties[current_format] = FormatProperties(format_props);
    }

    /* Retrieve device properties */
    vkGetPhysicalDeviceProperties(m_physical_device,
                                 &m_properties);

    /* Retrieve device queue data */
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device,
                                           &n_physical_device_queues, /* pCount                 */
                                            nullptr);                    /* pQueueFamilyProperties */

    if (n_physical_device_queues > 0)
    {
        std::vector<VkQueueFamilyProperties> queue_props;

        queue_props.resize(n_physical_device_queues);

        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device,
                                                &n_physical_device_queues,
                                                &queue_props[0]);

        for (uint32_t n_physical_device_queue = 0;
                      n_physical_device_queue < n_physical_device_queues;
                    ++n_physical_device_queue)
        {
            m_queue_families.push_back(QueueFamilyInfo(queue_props[n_physical_device_queue]) );
        }
    }

    /* Retrieve memory properties */
    vkGetPhysicalDeviceMemoryProperties(m_physical_device,
                                       &memory_properties);

    m_memory_properties.init(memory_properties);

    /* Retrieve device layers */
    result = vkEnumerateDeviceLayerProperties(m_physical_device,
                                             &n_physical_device_layers,
                                              nullptr); /* pProperties */
    anvil_assert_vk_call_succeeded(result);

    if (n_physical_device_layers > 0)
    {
        std::vector<VkLayerProperties> layer_props;

        layer_props.resize(n_physical_device_layers);

        result = vkEnumerateDeviceLayerProperties(m_physical_device,
                                                 &n_physical_device_layers,
                                                 &layer_props[0]);

        for (uint32_t n_layer = 0;
                      n_layer < n_physical_device_layers;
                    ++n_layer)
        {
            m_layers.push_back(Anvil::Layer(layer_props[n_layer]) );
        }
    }

    /* Retrieve device extensions */
    result = vkEnumerateDeviceExtensionProperties(m_physical_device,
                                                  nullptr, /* pLayerName */
                                                 &n_extensions,
                                                  nullptr); /* pProperties */
    anvil_assert_vk_call_succeeded(result);

    if (n_extensions > 0)
    {
        std::vector<VkExtensionProperties> extension_props;

        extension_props.resize(n_extensions);

        result = vkEnumerateDeviceExtensionProperties(m_physical_device,
                                                      nullptr, /* pLayerName */
                                                     &n_extensions,
                                                     &extension_props[0]);
        anvil_assert_vk_call_succeeded(result);

        for (uint32_t n_extension = 0;
                      n_extension < n_extensions;
                    ++n_extension)
        {
            m_extensions.push_back(Anvil::Extension(extension_props[n_extension]) );
        }
    }
}

/* Please see header for specification */
bool Anvil::PhysicalDevice::is_device_extension_supported(const char* extension_name) const
{
    return std::find(m_extensions.begin(),
                     m_extensions.end(),
                     extension_name) != m_extensions.end();
}

/* Please see header for specification */
bool Anvil::PhysicalDevice::is_layer_supported(const char* layer_name) const
{
    return std::find(m_layers .begin(),
                     m_layers.end(),
                     layer_name) != m_layers.end();
}

/* Please see header for specification */
void Anvil::PhysicalDevice::register_device(std::shared_ptr<Anvil::Device> device_ptr)
{
    auto device_iterator = std::find(m_cached_devices.begin(),
                                     m_cached_devices.end(),
                                     device_ptr);

    anvil_assert(device_iterator == m_cached_devices.end() );

    if (device_iterator == m_cached_devices.end() )
    {
        m_cached_devices.push_back(device_ptr);
    }
}

/* Please see header for specification */
void Anvil::PhysicalDevice::unregister_device(std::shared_ptr<Anvil::Device> device_ptr)
{
    auto device_iterator = std::find(m_cached_devices.begin(),
                                     m_cached_devices.end(),
                                     device_ptr);

    anvil_assert(device_iterator != m_cached_devices.end() );

    if (device_iterator != m_cached_devices.end() )
    {
        m_cached_devices.erase(device_iterator);
    }
}
