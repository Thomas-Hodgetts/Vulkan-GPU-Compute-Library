#pragma once
#include "vkGpuHandles.h"
#include <filesystem>
#include <fstream>

struct CalculatorData
{
	float m_FirstNumber = 0, m_SecondNumber = 0, m_Output = 0, m_Padding = 0;
	int m_Addition = 0, m_Subtraction = 0, m_Multiplication = 0, m_Division = 0;
};


void CalculatorApp();