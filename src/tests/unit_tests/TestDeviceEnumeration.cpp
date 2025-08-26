// 设备枚举测试代码
#include "AudioDeviceManager.h"
#include <gtest/gtest.h>

class DeviceEnumerationTest : public ::testing::Test {
protected:
	void SetUp() override {
		manager = new AudioDeviceManager();
		ASSERT_TRUE(manager->Initialize());
	}

	void TearDown() override {
		delete manager;
	}

	AudioDeviceManager* manager;
};

TEST_F(DeviceEnumerationTest, Initialization) {
	EXPECT_TRUE(manager->Initialize());
}

TEST_F(DeviceEnumerationTest, DeviceEnumeration) {
	EXPECT_TRUE(manager->PopulateDeviceLists());

	// 至少应该有一些设备
	auto physicalInputs = manager->GetPhysicalInputs();
	auto physicalOutputs = manager->GetPhysicalOutputs();

	EXPECT_FALSE(physicalInputs.empty() && physicalOutputs.empty());
}

TEST_F(DeviceEnumerationTest, DeviceClassification) {
	EXPECT_TRUE(manager->PopulateDeviceLists());

	auto physicalInputs = manager->GetPhysicalInputs();
	auto virtualInputs = manager->GetVirtualInputs();
	auto physicalOutputs = manager->GetPhysicalOutputs();
	auto virtualOutputs = manager->GetVirtualOutputs();

	// 检查设备分类是否正确
	for (const auto& device : physicalInputs) {
		EXPECT_EQ(device.type, DeviceType::Physical);
		EXPECT_EQ(device.direction, DeviceDirection::Input);
	}

	for (const auto& device : virtualInputs) {
		EXPECT_EQ(device.type, DeviceType::Virtual);
		EXPECT_EQ(device.direction, DeviceDirection::Input);
	}

	for (const auto& device : physicalOutputs) {
		EXPECT_EQ(device.type, DeviceType::Physical);
		EXPECT_EQ(device.direction, DeviceDirection::Output);
	}

	for (const auto& device : virtualOutputs) {
		EXPECT_EQ(device.type, DeviceType::Virtual);
		EXPECT_EQ(device.direction, DeviceDirection::Output);
	}
}
