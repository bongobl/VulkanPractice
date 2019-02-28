#pragma once
#include <vector>
#include <utility>
#include <vulkan/vulkan.h>
using namespace std;

typedef enum AdditionalQueueFlagBits {
	ADDITIONAL_VK_QUEUE_PRESENT_BIT = 0x00000020,
}AdditionalQueueFlagBits;

class QueueFamilyIndices{

	int doneCount;
	std::vector< pair<uint32_t, int> > indexMap;
public:
	
	bool compute(VkPhysicalDevice currPhysicalDevice);
	void addRequiredQueueType(uint32_t queueType);
	uint32_t getRequiredQueueTypeAt(unsigned int index) const;

private:

	void resetIndices();
	bool allHaveValues() const;
	bool hasValue(unsigned int index) const;
	void setQueueFamilyIndexAt(unsigned int index, int queueFamilyIndex);
	int getQueueFamilyIndexAt(unsigned int index) const;
	
};