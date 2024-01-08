struct CalculatorShader
{
    float m_FirstNumber, m_SecondNumber, m_Output, m_Padding;
    bool m_Addition, m_Subtraction, m_Multiplication, m_Division;
};

[[vk::binding(0, 0)]] RWStructuredBuffer<CalculatorShader> calculatorData;

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (calculatorData[DTid.x].m_Addition)
    {
        calculatorData[DTid.x].m_Output = calculatorData[DTid.x].m_FirstNumber + calculatorData[DTid.x].m_SecondNumber;
    }
    else if (calculatorData[DTid.x].m_Subtraction)
    {
        calculatorData[DTid.x].m_Output = calculatorData[DTid.x].m_FirstNumber - calculatorData[DTid.x].m_SecondNumber;
    }
    else if (calculatorData[DTid.x].m_Multiplication)
    {
        calculatorData[DTid.x].m_Output = calculatorData[DTid.x].m_FirstNumber * calculatorData[DTid.x].m_SecondNumber;
    }
    else if (calculatorData[DTid.x].m_Division)
    {
        calculatorData[DTid.x].m_Output = calculatorData[DTid.x].m_FirstNumber / calculatorData[DTid.x].m_SecondNumber;
    }
}