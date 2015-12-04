//:description In this proj are tested the following functionalities:
// can vector work properly if it is created with reserve and then overwritten
// by memcpy

#define THREADS_SORT_TIME

#include <iostream>
#include <windows.h>
#include <stdint.h>
#include <conio.h>
#include <vector>
#include <atomic>
#include <thread>
using namespace std;

//for x86_64 architecture
#ifdef _WIN32                //  Windows
#include <intrin.h>

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
    cout << "=== " << ExprText << " average == " << uiAverage / nCount << "\n"; \
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

struct Data{
    int * m_pAr;
    static int nSize;
    static HANDLE hEvent;    
};

HANDLE Data::hEvent;
int Data::nSize;


void VectorReserveTest();
void DeducePointerTypeFromVirtualFunc();

// tools used for threads/processes times test
void InMainSort(int * pAllAr, int nSize, int nTHREADS_COUNT);
void Init(int * pAr, int nSize);
void Show(int * pAr, int nSize);
void Sort(int * pAr, int nSize);
DWORD WINAPI ThreadFunc(void * pV);

// current tests
//#define T1_BothSortFunc(a) a;
#define T1_BothSortFunc(a)
void T1_Func(int * pForThreadsAr, int * pForMain, int nSize, int nTHREADS_COUNT);

int main()
{   
    srand((unsigned int) time(0));
    //VectorReserveTest();
    //DeducePointerTypeFromVirtualFunc();
    // return 0;

// //////////////////////////////////////////////////////////////////
// threads work

    int nTHREADS_COUNT, nSize;
    cout<<"Enter count of threads:"<<endl;
    cout<<"(The program will output performance from 1 to \"count \" of threads)"<<endl;
    cin>>nTHREADS_COUNT;
    cout<<"Enter array size (portion for 1 thread):"<<endl;
    cout<<"(nTotalArraySize = nSize * nThreadsCount)"<<endl;
    cin>>nSize;


    //HANDLE hStartProcEv = CreateEvent(0, TRUE, FALSE, L"ToStartAllProcesses");
    //SetEvent(hStartProcEv);

    Data::nSize = nSize;
    Data::hEvent = CreateEvent(NULL, TRUE, FALSE, L"To_start_THREADS_COUNT_threads");
    cout<<"Your CPU has "<<std::thread::hardware_concurrency()<<" kernels "<<endl;

    for(int i = 0; i < nTHREADS_COUNT; ++i)
    {
        char pText[8] = "threads";
        if(0 == i)
            pText[6] = 0;
        cout<<"====== performance of "<<i + 1<<" "<<pText <<" =============="<<endl;
        int * pForMain      = new int[nSize * nTHREADS_COUNT];
        int * pForThreadsAr = new int[nSize * nTHREADS_COUNT];
        Init(pForThreadsAr, nSize * nTHREADS_COUNT);
        memcpy(pForMain, pForThreadsAr, sizeof(int) * nSize * nTHREADS_COUNT);

                                                                                        T1_BothSortFunc(T1_Func(pForThreadsAr, pForMain, nSize, nTHREADS_COUNT); break;)
        Data * pDataAr = new Data[nTHREADS_COUNT];
        HANDLE * phThAr = new HANDLE[nTHREADS_COUNT];
        for(int j = 0; j < nTHREADS_COUNT; ++j)
        {
            pDataAr[j].m_pAr = pForThreadsAr + nSize * j;
            phThAr[j] = CreateThread(0, 0, ThreadFunc, pDataAr + j, 0, 0);
        }
        int nCount = 1;
        unsigned long long dStartTime, dFinTime, dThDiff,  dMainSortTime;

        EXPR_TIME_CONS_AVER_OUT(nCount, InMainSort(pForMain, nSize, nTHREADS_COUNT), false, dMainSortTime);
        //cout<<"dMainSortTime = "<<dMainSortTime<<endl;

        // time before threads start
        cout<<"To start "<<i+1<<" "<<pText<<" - press any key"<<endl;
        if(0 == i)
            _getch();
        SetEvent(Data::hEvent);

        dStartTime = rdtsc();
        WaitForMultipleObjects(nTHREADS_COUNT, phThAr, TRUE, INFINITE);
        dFinTime = rdtsc();
        dThDiff = dFinTime - dStartTime;
        if(dThDiff > dMainSortTime)
            cout<<" main is faster  on "<<dThDiff - dMainSortTime<<" ticks"<<endl;
        else
            cout<<" threads are faster on "<<dMainSortTime - dThDiff<<" ticks"<<endl;
        cout<<"ticks per data portion = "<<dThDiff<<" / "<<nSize<<" = "<< dThDiff / (double)nSize <<endl;
        cout<<"total "<<i + 1<<" "<<pText<<" time =  "<<dThDiff<<endl;



        delete [] pForMain;
        delete [] pForThreadsAr;
        delete [] pDataAr;
        for(int j = 0; j < nTHREADS_COUNT; ++j)
            CloseHandle(phThAr[j]);
        delete [] phThAr;
    }

    return 0;
}


void VectorReserveTest()
{
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
}
void DeducePointerTypeFromVirtualFunc()
{
    // task: how to call virtual function to let callee know,
    // from what pointer was call made.
#ifdef DEDUCE_POINTER_TYPE_FROM_VIRTFUNC
    C c;
    A * pA = &c;
    B * pB = &c;
    pA->Func();
    pB->Func();
#endif
}


void InMainSort(int * pAllAr, int nSize, int nTHREADS_COUNT)
{
    // this function sorts Total array, intended for THREADS_COUNT threads
    // dividing it on THREADS_COUNT parts, not to move too much elements
    // imitating division of all job on several threads

    // nSize - size of ONE portion of Total array (one thread job)
    // total memory was created as nSize * nTHREADS_COUNT elements.
    // not to deal with possible remainder of division:
    // nSize = nTotalSize / nTHREADS_COUNT;

    int nMin, iMinInd;
    int * pAr;  // current array
    for(int i0 = 0; i0 < nTHREADS_COUNT; ++i0)
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

DWORD WINAPI ThreadFunc(void * pV)
{
    Data * pData = (Data*)pV;
    WaitForSingleObject(pData->hEvent, INFINITE);
    Sort(pData->m_pAr, pData->nSize);

    return 0;
}
void Init(int * pAr, int nSize)
{
    for(int i = 0; i < nSize; ++i)
        pAr[i] = rand() % 100;
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
// //////////////////////////  TESTS
void T1_Func(int * pForThreadsAr, int * pForMain, int nSize, int nTHREADS_COUNT)
{
    cout<<"==== Sort :"<<endl;
    Sort(pForThreadsAr, nSize * nTHREADS_COUNT);
    Show(pForThreadsAr, nSize * nTHREADS_COUNT);
    cout<<"==== InMainSort :"<<endl;
    InMainSort(pForMain, nSize, nTHREADS_COUNT);
    Show(pForMain, nSize * nTHREADS_COUNT);
}

// end of TESTS
