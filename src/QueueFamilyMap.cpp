#include <QueueFamilyMap.h>


bool QueueFamilyMap::compute(VkPhysicalDevice currPhysicalDevice, VkSurfaceKHR surface){

	resetIndices();

	uint32_t queueFamilyCount;

	// Retrieve all queue families.
    vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &queueFamilyCount, NULL);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &queueFamilyCount, queueFamilies.data());


    // Now find a single queue family that supports all our required operations (not ideal way to do this)
    uint32_t currFamilyIndex;
    for (currFamilyIndex = 0; currFamilyIndex < queueFamilies.size(); ++currFamilyIndex) {
        VkQueueFamilyProperties currFamily = queueFamilies[currFamilyIndex];

		if (currFamily.queueCount > 0) {

			for (unsigned int currMapIndex = 0; currMapIndex < indexMap.size(); ++currMapIndex) {
				if (!hasValue(currMapIndex)) {

					//Special way to check if queue family has present support
					if (getRequiredQueueTypeAt(currMapIndex) == ADDITIONAL_VK_QUEUE_PRESENT_BIT) {

						VkBool32 presentSupport = false;
						vkGetPhysicalDeviceSurfaceSupportKHR(currPhysicalDevice, currFamilyIndex, surface, &presentSupport);
						if (presentSupport) {
							setQueueFamilyIndexAt(currMapIndex, currFamilyIndex);
						}
					}

					//Standard way to check if queue family supports this type of operation
					else if (currFamily.queueFlags & getRequiredQueueTypeAt(currMapIndex)) {
						setQueueFamilyIndexAt(currMapIndex, currFamilyIndex);
					}
					if (allHaveValues()) {
						return true;
					}
				}
			}
		}
    }

	return allHaveValues(); 
}

void QueueFamilyMap::addRequiredQueueType(uint32_t queueType){

	pair<uint32_t, int> elem;
	elem.first = queueType;
	elem.second = -1;
	indexMap.push_back(elem);
}

uint32_t QueueFamilyMap::getRequiredQueueTypeAt(unsigned int index) const{
	return indexMap.at(index).first;
}

int QueueFamilyMap::numRequired() const {
	return (int)indexMap.size();
}

//Private Helpers for class QueueFamilyIndices
void QueueFamilyMap::resetIndices(){
	doneCount = 0;

	for(unsigned int i = 0; i < indexMap.size(); ++i){
		indexMap.at(i).second = -1;
	}
}
bool QueueFamilyMap::allHaveValues() const{
	return doneCount >= indexMap.size();
}

bool QueueFamilyMap::hasValue(unsigned int index) const{
	return indexMap.at(index).second != -1;
}


void QueueFamilyMap::setQueueFamilyIndexAt(unsigned int index, int queueFamilyIndex){

	indexMap.at(index).second = queueFamilyIndex;
	++doneCount;
}

int QueueFamilyMap::getQueueFamilyIndexAt(unsigned int index) const{
	return indexMap.at(index).second;
}

