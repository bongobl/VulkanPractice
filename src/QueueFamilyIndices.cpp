#include <QueueFamilyIndices.h>


bool QueueFamilyIndices::compute(VkPhysicalDevice currPhysicalDevice){

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

			for (int i = 0; i < indexMap.size(); ++i) {
				if (!hasValue(i) && (currFamily.queueFlags & getRequiredQueueTypeAt(i))) {
					setQueueFamilyIndexAt(i,currFamilyIndex);
					
					if (allHaveValues()) {
						return true;
					}
				}
			}
		}
    }

	return allHaveValues(); 
}

void QueueFamilyIndices::addRequiredQueueType(uint32_t queueType){

	pair<uint32_t, int> elem;
	elem.first = queueType;
	elem.second = -1;
	indexMap.push_back(elem);
}

uint32_t QueueFamilyIndices::getRequiredQueueTypeAt(unsigned int index) const{
	return indexMap.at(index).first;
}

//Private Helpers for class QueueFamilyIndices
void QueueFamilyIndices::resetIndices(){
	doneCount = 0;

	for(unsigned int i = 0; i < indexMap.size(); ++i){
		indexMap.at(i).second = -1;
	}
}
bool QueueFamilyIndices::allHaveValues() const{
	return doneCount >= indexMap.size();
}

bool QueueFamilyIndices::hasValue(unsigned int index) const{
	return indexMap.at(index).second != -1;
}


void QueueFamilyIndices::setQueueFamilyIndexAt(unsigned int index, int queueFamilyIndex){

	indexMap.at(index).second = queueFamilyIndex;
	++doneCount;
}


int QueueFamilyIndices::getQueueFamilyIndexAt(unsigned int index) const{
	return indexMap.at(index).second;
}