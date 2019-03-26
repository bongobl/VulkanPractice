#pragma once
#include <vector>
#include <utility>
#include <vulkan/vulkan.h>
using namespace std;

typedef enum AdditionalQueueFlagBits {
	ADDITIONAL_VK_QUEUE_PRESENT_BIT = 0x00000020,
}AdditionalQueueFlagBits;

class QueueFamilyMap{

	unsigned int doneCount;

	//maps required queue type enums to their queue family index
	std::vector< pair<uint32_t, int> > indexMap;
public:

	//Note: Surface only needed if using present queue
	void compute(VkPhysicalDevice currPhysicalDevice, VkSurfaceKHR surface);	
	void addRequiredQueueType(uint32_t queueType);
	int getQueueFamilyIndexAt(unsigned int index) const;
	int numRequired() const;
	bool allHaveValues() const;
	
private:

	void resetIndices();
	
	bool hasValue(unsigned int index) const;
	void setQueueFamilyIndexAt(unsigned int index, int queueFamilyIndex);
	uint32_t getRequiredQueueTypeAt(unsigned int index) const;
		
};