#include "CalculatorApp.h"

void CalculatorApp()
{
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

		std::cout << "Select Mode +,-,*,/: ";
		std::cin >> modeInput;
		modeInput.resize(1);

		switch (modeInput[0])
		{
		default:
			inputData.m_Addition = false;
			inputData.m_Subtraction = false;
			inputData.m_Multiplication = false;
			inputData.m_Division = false;
			break;
		case '+':
			inputData.m_Addition = true;
			inputData.m_Subtraction = false;
			inputData.m_Multiplication = false;
			inputData.m_Division = false;
			break;
		case '-':
			inputData.m_Addition = false;
			inputData.m_Subtraction = true;
			inputData.m_Multiplication = false;
			inputData.m_Division = false;
			break;
		case '*':
			inputData.m_Addition = false;
			inputData.m_Subtraction = false;
			inputData.m_Multiplication = true;
			inputData.m_Division = false;
			break;
		case '/':
			inputData.m_Addition = false;
			inputData.m_Subtraction = false;
			inputData.m_Multiplication = false;
			inputData.m_Division = true;
			break;
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