#pragma once
#include <vector>
#include <utility>
#include <vulkan/vulkan.h>
using namespace std;

typedef enum AdditionalQueueFlagBits {
	ADDITIONAL_VK_QUEUE_PRESENT_BIT = 0x00000020,
}AdditionalQueueFlagBits;

class QueueFamilyMap{

	static unsigned int doneCount;

	//maps required queue type enums to their queue family index
	static std::vector< pair<uint32_t, int> > indexMap;
public:

	//Note: Surface only needed if using present queue
	static void compute(VkPhysicalDevice currPhysicalDevice, VkSurfaceKHR surface);	
	static void addRequiredQueueType(uint32_t queueType);
	static int getQueueFamilyIndexAt(unsigned int index);
	static int numRequired();
	static bool allHaveValues();
	
private:

	static void resetIndices();
	
	static bool hasValue(unsigned int index);
	static void setQueueFamilyIndexAt(unsigned int index, int queueFamilyIndex);
	static uint32_t getRequiredQueueTypeAt(unsigned int index);
		
};