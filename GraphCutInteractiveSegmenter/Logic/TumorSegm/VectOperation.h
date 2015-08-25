#ifndef VECTOPERATION_H_INCLUDED
#define VECTOPERATION_H_INCLUDED

#include <vector>
#include <numeric>
#include <algorithm>

using namespace std;

//operations for vector
template<typename T>
vector<T> operator+(const vector<T>& a, const T& scalar)
{
    vector<T> result(a.size());
    for(unsigned int i=0; i<a.size(); i++)
        {
            result[i] = a[i] + scalar;
        }
    return result;
}

template<typename T>
vector<T> operator-(const vector<T>& a, const T& scalar)
{
    vector<T> result(a.size());
    for(unsigned int i=0; i<a.size(); i++)
        {
            result[i] = a[i] - scalar;
        }
    return result;
}

template<typename T>
vector<T> operator-(const vector<T>& a)
{
    vector<T> result(a.size());
    for(unsigned int i=0; i<a.size(); i++)
        {
            result[i] = -a[i];
        }
    return result;
}

template<typename T>
vector<T> operator*(const vector<T>& a, const T& scalar)
{
    vector<T> result(a.size());
    for(unsigned int i=0; i<a.size(); i++)
        {
            result[i] = a[i]*scalar;
        }
    return result;
}

template<typename T>
vector<T> operator/(const vector<T>& a, T dividor)
{
    assert(dividor!=0);
    vector<T> result(a);
    for(int i=0; i<result.size(); i++)
        {
            result[i] = result[i]/dividor;
        }

    return result;
}

template<typename T>
T min(const vector<T>& a)
{
    return *std::min_element(a.begin(),a.end());
}

template<typename T>
T max(const vector<T>& a)
{
    return *std::max_element(a.begin(),a.end());
}
#endif // VECTOPERATION_H_INCLUDED
