#include "vkGpuHandles.h"
#include <filesystem>
#include <fstream>

struct CalculatorData
{
	float m_FirstNumber = 0, m_SecondNumber = 0, m_Output = 0, m_Padding = 0;
	bool m_Addition = false, m_Subtraction = false, m_Multiplication = false, m_Division = false;
};


void GenerateAndRunShaderCompile()
{

	std::filesystem::path basePath = std::filesystem::current_path();

	//create new shader compile file
	std::ofstream outFile(basePath.string() + "/ShaderCompile.bat");

	//sort which files need to be compiled
	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(basePath))
	{
		char fileExtension[4];
		std::string filePath = entry.path().string();
		filePath.copy(fileExtension, 4, filePath.size() - 4);
		//std::cout << fileExtension << std::endl;
		if (fileExtension[0] == 'h' && fileExtension[1] == 'l' && fileExtension[2] == 's' && fileExtension[3] == 'l')
		{
			//Generate compile command
			std::string newName = filePath;
			newName[newName.size() - 4] = 's';
			newName[newName.size() - 3] = 'p';
			newName[newName.size() - 2] = 'v';
			newName.pop_back();

			std::string outputText = "C:/VulkanSDK/1.3.246.1/Bin/dxc.exe -spirv -T cs_6_0 -E main " + filePath + " - Fo " + newName;
			outFile << outputText << std::endl;
			int arrSize = filePath.size() - 1;
		}
	}
	//outFile << "pause" << std::endl;

	//close and run the file
	outFile.close();
	system("ShaderCompile.bat");
}


int main()
{
	//GenerateAndRunShaderCompile();


	std::cout << "Test Calculator App" << std::endl;

	// System Set up
	vkGenerics::vkQueueFamilyBit bit = vkGenerics::vkQueueFamilyBit::BASE;
	bit |= vkGenerics::vkQueueFamilyBit::COMPUTE;
	uint32_t memorySize = (sizeof(CalculatorData));
	CalculatorData inputData;
	VkExecuteHint hint = VkExecuteHint();


	GPUHandle gpuHandle(GPUHandleMode::COMPUTE, std::vector<const char*>(), std::vector<const char*>());
	GPUMemoryHandle gpuMemoryHandle = GPUMemoryHandle(&gpuHandle, memorySize);
	GPUKernel calculatorMathKernel = GPUKernel(&gpuHandle, L"CalculatorShader.spv", "main");
	GPUMemoryAdapter memoryAdapter = GPUMemoryAdapter(&gpuHandle, 1);
	HostAccessablePtr calculatorMemory = gpuMemoryHandle.WriteDataToCpuVisablePool(&inputData, memorySize);
	memoryAdapter.AttachMemory(calculatorMemory);
	calculatorMathKernel.LinkMemory(&memoryAdapter, VK_PIPELINE_BIND_POINT_COMPUTE);

	//Operation Code
	bool InOperation = true;
	while (InOperation)
	{
		std::string modeInput;

		std::cout << "Select Mode +, -,*,/: ";
		std::cin >> modeInput;
		modeInput.resize(1);

		switch (modeInput[0])
		{
		default:
			inputData.m_Addition = 0;
			inputData.m_Subtraction = 0;
			inputData.m_Multiplication = 0;
			inputData.m_Division = 0;
			break;
		case '+':
			inputData.m_Addition = 1;
			inputData.m_Subtraction = 0;
			inputData.m_Multiplication = 0;
			inputData.m_Division = 0;
		case '-':
			inputData.m_Addition = 0;
			inputData.m_Subtraction = 1;
			inputData.m_Multiplication = 0;
			inputData.m_Division = 0;
		case '*':
			inputData.m_Addition = 0;
			inputData.m_Subtraction = 0;
			inputData.m_Multiplication = 1;
			inputData.m_Division = 0;
		case '/':
			inputData.m_Addition = 0;
			inputData.m_Subtraction = 0;
			inputData.m_Multiplication = 0;
			inputData.m_Division = 1;
		}

		std::cout << "First Number: ";
		std::cin >> inputData.m_FirstNumber;
		std::cout << "" << std::endl;
		std::cout << "Second Number: ";
		std::cin >> inputData.m_SecondNumber;
		std::cout << "" << std::endl;

		memoryAdapter.WriteData(&inputData);
		calculatorMathKernel.RequestExecution(&gpuHandle, hint);
		gpuHandle.ExecuteCompute();
		memoryAdapter.ReadData(&inputData, memorySize);

		std::cout << inputData.m_Output << std::endl;

		std::cout << "Finished with application, type 'exit' to exit and press enter or hit enter to continue." << std::endl;
		std::string exitCondition;
		std::cin >> exitCondition;
		if (exitCondition == "Exit" || exitCondition == "exit")
		{
			InOperation = false;
		}
	}
}