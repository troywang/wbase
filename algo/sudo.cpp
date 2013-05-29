/*
 * sudo.cpp
 *
 *  Created on: May 29, 2013
 *      Author: king
 */
#include<iostream>
#include<string.h>
using namespace std;


int temp[9 * 9];
int data[9 * 9];
#define TEMP(i, j, k, l) temp[27 * i + 9 * j + 3 * k + l]
#define DATA(i, j, k, l) data[27 * i + 9 * j + 3 * k + l]

int fill_curr(int i, int j, int k, int l)
{
	int map[10];
	memset(map, 0, sizeof(map));
	for (int ii = 0; ii < 3; ii++)
	{
		for (int jj = 0; jj < 3; jj++)
		{
			map[DATA(i, j, ii, jj)] = 1;
			map[DATA(i, ii, k, jj)] = 1;
			map[DATA(ii, j, jj, l)] = 1;
		}
	}

	for (int ii = 0; ii < 10; ii++)
	{
		if (map[ii] == 0 && ii > DATA(i, j, k, l))
		{
			DATA(i, j, k, l) = ii;
			return ii;
		}
	}

	//backtrack
	DATA(i, j, k, l) = 0;
	return -1;
}

struct point
{
	int i;
	int j;
	int k;
	int l;
};

struct point forward(int i, int j, int k, int l)
{
	int m = j;
	int n = l + 1;
	struct point p;
	p.i = -1;
	for (int ii = i; ii < 3; ii++)
	{
		for (int jj = m; jj < 3; jj++)
		{
			for (int kk = k; kk < 3; kk++)
			{
				for (int ll = n; ll < 3; ll++)
				{
					if (TEMP(ii, jj, kk, ll) == 0) {
						p.i = ii;
						p.j = jj;
						p.k = kk;
						p.l = ll;
						//cout << "-->(" << p.i << "," << p.j << "," << p.k << "," << p.l << ")=";
						return p;
					}
				}
				n = 0;
			}
			k = 0;
		}
		m = 0;
	}
	return p;
}

struct point back(int i, int j, int k, int l)
{
	int m = j;
	int n = l - 1;
	struct point p;
	p.i = -1;
	for (int ii = i; ii >= 0; ii--)
	{
		for (int jj = m; jj >= 0; jj--)
		{
			for (int kk = k; kk >=0; kk--)
			{
				for (int ll = n; ll >= 0; ll--)
				{
					if (TEMP(ii, jj, kk, ll) == 0)
					{
						p.i = ii;
						p.j = jj;
						p.k = kk;
						p.l = ll;
						//cout << "<--(" << p.i << "," << p.j << "," << p.k << "," << p.l << ")=";
						return p;
					}
				}
				n = 2;
			}
			k = 2;
		}
		m = 2;
	}
	return p;
}

int main()
{
	memset(data, 0, sizeof(data));
	memset(temp, 0, sizeof(temp));
	TEMP(0, 0, 0, 0) = 8;
	TEMP(0, 0, 1, 2) = 3;
	TEMP(0, 0, 2, 1) = 7;

	TEMP(0, 1, 1, 0) = 6;
	TEMP(0, 1, 2, 1) = 9;

	TEMP(0, 2, 2, 0) = 2;

	TEMP(1, 0, 0, 1) = 5;

	TEMP(1, 1, 0, 2) = 7;
	TEMP(1, 1, 1, 1) = 4;
	TEMP(1, 1, 1, 2) = 5;
	TEMP(1, 1, 2, 0) = 1;

	TEMP(1, 2, 1, 0) = 7;
	TEMP(1, 2, 2, 1) = 3;

	TEMP(2, 0, 0, 2) = 1;
	TEMP(2, 0, 1, 2) = 8;
	TEMP(2, 0, 2, 1) = 9;

	TEMP(2, 1, 1, 0) = 5;

	TEMP(2, 2, 0, 1) = 6;
	TEMP(2, 2, 0, 2) = 8;
	TEMP(2, 2, 1, 1) = 1;
	TEMP(2, 2, 2, 0) = 4;

	memcpy(data, temp, sizeof(data));

	struct point cur = {0, 0, 0, -1};
	struct point prev;
	while (true) {
		prev = cur;
		cur = forward(cur.i, cur.j, cur.k, cur.l);

		if (cur.i == -1) {
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
				{
					for (int k = 0; k < 3; k++)
						for (int l = 0; l < 3; l++)
							cout << DATA(i, k, j, l);
					cout << endl;
				}
			cur = prev;
			cur = back(cur.i, cur.j, cur.k, cur.l);
		}

		while (fill_curr(cur.i, cur.j, cur.k, cur.l) == -1) {
			cur = back(cur.i, cur.j, cur.k, cur.l);
			if (cur.i == -1) {
				cout << "failed" << endl;
				return -1;
			}
		}
	}

	return 0;
}






