#include<cstdio>
#include<iostream>
#include<cstring>
#include<algorithm>

using namespace std;

void adjust(int arr[], int len, int index)
{
    int left = 2*index + 1;
    int right = 2*index + 2;
    int maxIdx = index;
    if(left<len && arr[left] > arr[maxIdx]) maxIdx = left;
    if(right<len && arr[right] > arr[maxIdx]) maxIdx = right;  // maxIdx是3个数中最大数的下标
    if(maxIdx != index)                 // 如果maxIdx的值有更新
    {
        swap(arr[maxIdx], arr[index]);
        adjust(arr, len, maxIdx);       // 递归调整其他不满足堆性质的部分
    }

}


void heapSort(int arr[], int size)
{
    for(int i=size/2 - 1; i >= 0; i--)  // 对每一个非叶结点进行堆调整(从最后一个非叶结点开始)
    {
        adjust(arr, size, i);
    }
    for(int i = size - 1; i >= 1; i--)
    {
        swap(arr[0], arr[i]);           // 将当前最大的放置到数组末尾
        adjust(arr, i, 0);              // 将未完成排序的部分继续进行堆排序
    }
}


int main()
{
    int array[8] = {8, 1, 14, 3, 21, 5, 7, 10};
    heapSort(array, 8);
    for(auto it: array)
    {
        cout<<it<<endl;
    }
    return 0;
}
