//:description In this proj are tested the following functionalitues:
// can vector work properly if it is created with reserve and then overwritten
// by memcpy
//#define VECTOR_RESERVE_TEST
#define THREADS_SORT_TIME

#include <iostream>
#include <windows.h>
#include <stdint.h>
#include <conio.h>
#include <vector>
using namespace std;
using std::cout;
using std::cin;
using std::endl;



#include <atomic>
#include <thread>
//#include <assert.h>
//std::atomic<bool> x, y;
//std::atomic<int> z;

//for x86_64 architecture
#ifdef _WIN32                //  Windows
#include <intrin.h>
//uint64_t rdtsc()
//{
//	return __rdtsc();
//}
unsigned long long rdtsc()
{
    return __rdtsc();
}
#else                        //  Linux/GCC
uint64_t rdtsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#endif //_WIN32

#define EXPR_TIME_CONS_AVER_OUT(nCount, expr, bShowCurTime, outV)\
{\
    char ExprText[sizeof(#expr)] = { 0 }; \
    memcpy(ExprText, #expr, sizeof(#expr)); \
    if (bShowCurTime == true)\
        cout << "=== " << ExprText << " === start" << endl; \
        unsigned long long ui1, ui2, uiTicks, uiAverage = 0; \
    for (int iIn = 0; iIn < nCount; ++iIn)\
    {\
        ui1 = rdtsc(); \
        expr; \
        ui2 = rdtsc(); \
        uiTicks = ui2 - ui1; \
        uiAverage += uiTicks; \
        if (bShowCurTime == true)\
            cout << uiTicks << endl; \
    }\
    outV = uiAverage / nCount; \
    cout << "=== " << ExprText << " average == " << uiAverage / nCount << "\n\n"; \
}


#ifdef DEDUCE_POINTER_TYPE_FROM_VIRTFUNC
class A
{
    int m_a;
public :
    void Func()
    {
        cout<<"A::Func()"<<endl;
        Show("from A*");
    }
    virtual void Show(const char * pStr)
    {
        cout<<"A::Show() "<<endl;
    }
};

class B
{
    int m_b;
public :
    void Func()
    {
        cout<<"B::Func()"<<endl;
        Show("from B*");
    }
    virtual void Show(const char * pStr)
    {
        cout<<"B::Show()"<<endl;
    }
};
class C : public A, public B
{
    int m_c;
public:
    virtual void Show(const char * pStr)
    {
        cout<<"C::Show()"<<endl;
        cout<<"Was called : "<<pStr<<endl;
    }
};
#endif

void Init(int * pAr, int nSize)
{
    for(int i = 0; i < nSize; ++i)
        pAr[i] = rand() % 100;//* rand();
}

void Show(int * pAr, int nSize)
{
    for(int i = 0; i < nSize; ++i)
        cout<<pAr[i]<<' ';
    cout<<endl;
}

void Sort(int * pAr, int nSize)
{
    int nMin, iMinInd;
    for(int i = 0; i < nSize; ++i)
    {
        nMin = pAr[i];
        iMinInd = i;
        for(int j = i + 1; j < nSize; ++j)
            if(pAr[j] < nMin)
            {
                nMin = pAr[j];
                iMinInd = j;
            }
        pAr[iMinInd] = pAr[i];
        pAr[i] = nMin;
    }
}

#define TIMES_AR_SIZE 10
struct Data{
    int * m_pAr;
    static int nSize;
    static HANDLE hEvent;
    uint64_t uTimeAr[TIMES_AR_SIZE];
};

HANDLE Data::hEvent;
int Data::nSize;

DWORD WINAPI ThreadFunc(void * pV)
{
    Data * pData = (Data*)pV;
    WaitForSingleObject(pData->hEvent, INFINITE);
    pData->uTimeAr[0] = rdtsc();
    Sort(pData->m_pAr, pData->nSize);
    pData->uTimeAr[1] = rdtsc();

    return 0;
}
#define THREADS_COUNT 4

// this function sorts Total array, intended for THREADS_COUNT threads
// dividing it on THREADS_COUNT parts, not to move too much elements
// imitating division of all job on several threads
void InMainSort(int * pAllAr, int nSize)
{
    // nSize - size of ONE portion of Total array (one thread job)
    int nMin, iMinInd;
    int * pAr;  // current array
    for(int i0 = 0; i0 < THREADS_COUNT; ++i0)
    {
        pAr = pAllAr + nSize * i0;
        for(int i = 0; i < nSize; ++i)
        {
            nMin = pAr[i];
            iMinInd = i;
            for(int j = i + 1; j < nSize; ++j)
                if(pAr[j] < nMin)
                {
                    nMin = pAr[j];
                    iMinInd = j;
                }
            pAr[iMinInd] = pAr[i];
            pAr[i] = nMin;
        }
    }
}


int main()
{   
#ifdef VECTOR_RESERVE_TEST
    int nCount;
    cout<<"Enter vector size"<<endl;
    cin>>nCount;
    int * pAr = new int[nCount];
    for(int i = 0; i < nCount; ++i)
        pAr[i] = rand() % 31;
    for(int i = 0; i < nCount; ++i)
        cout<<pAr[i]<<' ';
    cout<<endl;

    vector<int> vc;
    vc.resize(nCount);
    memcpy(&vc[0], pAr, sizeof(int)* nCount);
    delete [] pAr;

    for(int i = 0; i < 3; ++i)
        vc.push_back(i + 1);
    for(int i = 0, nSize = vc.size(); i < nSize; ++i)
        cout<<vc[i]<<' ';
    cout<<endl;

    return 0;
#endif
    // task: how to call virtual function to let callee know,
    // from what pointer was call made.
//    C c;
//    A * pA = &c;
//    B * pB = &c;
//    pA->Func();
//    pB->Func();
    // ////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// threads work
#ifdef THREADS_SORT_TIME
    int nSize;
    cout<<"Enter size "<<endl;
    cin>>nSize; //20000
    Data::nSize = nSize;

    //int * pForMain      = new int[nSize * THREADS_COUNT];
    int * pForThreadsAr = new int[nSize * THREADS_COUNT];
    Init(pForThreadsAr, nSize * THREADS_COUNT);
    //memcpy(pForMain, pForThreadsAr, sizeof(int) * nSize * THREADS_COUNT);
    HANDLE hStartProcEv = CreateEvent(0, TRUE, FALSE, L"ToStartAllProcesses");
    //SetEvent(hStartProcEv);

// COMPARING TWO SORT FUNCTIONS
//    int nCount = 1;
//    unsigned long long dAverTime1, dAverTime2;
//    EXPR_TIME_CONS_AVER_OUT(nCount, Sort(pForThreadsAr, nSize * THREADS_COUNT), true, dAverTime1);
//    cout<<"1 average time = "<<dAverTime1<<endl;
//    EXPR_TIME_CONS_AVER_OUT(nCount, InMainSort(pForMain, nSize), true, dAverTime2);
//    cout<<"2 average time = "<<dAverTime2<<endl;
//    if(dAverTime1 > dAverTime2)
//        cout<<" 1 > 2  "<<dAverTime1 - dAverTime2<<endl;
//    else
//        cout<<" 1 <= 2  "<<dAverTime2 - dAverTime1<<endl;

// COMPARING Sort in main with sort dividing array on THREADS_COUNT parts

    Data::hEvent = CreateEvent(NULL, TRUE, FALSE, L"To_start_THREADS_COUNT_threads");
    Data * pDataAr = new Data[THREADS_COUNT];
    HANDLE * phThAr = new HANDLE[THREADS_COUNT];
    for(int i = 0; i < THREADS_COUNT; ++i)
    {
        pDataAr[i].m_pAr = pForThreadsAr + nSize * i;
        phThAr[i] = CreateThread(0, 0, ThreadFunc, pDataAr +i, 0, 0);
    }
    //int nCount = 1;
    unsigned long long dStartTime, dFinTime, dThDiff,  dMainSortTime;

//    EXPR_TIME_CONS_AVER_OUT(nCount, InMainSort(pForMain, nSize), true, dMainSortTime);
//    cout<<"dMainSortTime = "<<dMainSortTime<<endl;

    // time before threads start
    dStartTime = rdtsc();
    SetEvent(Data::hEvent);
    WaitForMultipleObjects(THREADS_COUNT, phThAr, TRUE, INFINITE);
    dFinTime = rdtsc();
    dThDiff = dFinTime - dStartTime;
//    if(dThDiff > dMainSortTime)
//        cout<<" main is faster "<<dThDiff - dMainSortTime<<endl;
//    else
//        cout<<" threads are faster  "<<dMainSortTime - dThDiff<<endl;

    cout<<"ticks per int = "<<dThDiff / nSize <<endl;
    cout<<"total "<<THREADS_COUNT<<" threads time =  "<<dThDiff<<endl;



    //delete [] pForMain;
    delete [] pForThreadsAr;
    delete [] pDataAr;
    delete [] phThAr;

    // /////////////////////////////
#endif //THREADS_SORT_TIME
//    int nCount = 10;
//    unsigned long long dAverTime;
//    EXPR_TIME_CONS_AVER_OUT(nCount, cout << "Hello World!" << endl, true, dAverTime);
//    cout<<"average time = "<<dAverTime<<endl;
    return 0;
}

