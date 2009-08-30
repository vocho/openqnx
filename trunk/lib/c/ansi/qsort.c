/* qsort function */
#include <stdlib.h>
#include <string.h>
#include <yvals.h>
_STD_BEGIN

#define BUF_SIZE	256	/* > 0, auto buffer size */
#define SORT_MAX	32	/* > 1, cutover for insertion sort */
#define ASGN_MAX	64	/* >= 0, cutover for memcpy */

static void swap(char *qb, char *qe, size_t size)
	{	/* swap elements *qb and *qe */
	if (size < ASGN_MAX)
		{	/* copy byte by byte for small objects */
		unsigned int i;

		for (i = 0; i < size; ++i)
			{	/* copy a byte */
			char tmp = qb[i];

			qb[i] = qe[i];
			qe[i] = tmp;
			}
		}
	else
		{	/* memcpy for large objects */
		char buf[BUF_SIZE];	/* chunk to copy on swap */
		size_t ms;

		for (ms = size; 0 < ms; )
			{	/* swap as many as possible */
			size_t m = ms < sizeof (buf) ? ms : sizeof (buf);
			memcpy(buf, qb, m);
			memcpy(qb, qe, m);
			memcpy(qe, buf, m);
			ms -= m, qb += m, qe += m;
			}
		}
	}

static void rot3(char *qf, char *qm, char *ql, size_t size)
	{	/* right rotate, *qf -> *qm ->*ql -> *qf */
	if (size < ASGN_MAX)
		{	/* copy byte by byte for small objects */
		unsigned int i;

		for (i = 0; i < size; ++i)
			{	/* right rotate once */
			char tmp = qf[i];

			qf[i] = ql[i];
			ql[i] = qm[i];
			qm[i] = tmp;
			}
		}
	else
		{	/* memcpy for large objects */
		char buf[BUF_SIZE];	/* chunk to copy on swap */
		size_t ms;

		for (ms = size; 0 < ms; )
			{	/* rotate as many as possible */
			size_t m = ms < sizeof (buf) ? ms : sizeof (buf);

			memcpy(buf, qf, m);
			memcpy(qf, ql, m);
			memcpy(ql, qm, m);
			memcpy(qm, buf, m);
			ms -= m, qf += m, qm += m, ql += m;
			}
		}
	}

static void rotate(char *qb, char *qe, size_t size)
	{	/* right rotate [*qb, *qe] one place */
	char buf[BUF_SIZE];	/* chunk to copy on rotate */
	size_t ms, mtot = (qe - qb) + size;

	for (ms = size; 0 < ms; )
		{	/* rotate as many as possible */
		size_t m = ms < sizeof (buf) ? ms : sizeof (buf);

		memcpy(buf, qe + (size - m), m);
		memmove(qb + m, qb, mtot - m);
		memcpy(qb, buf, m);
		ms -= m;
		}
	}

static void med3(char *qf, char *qm, char *ql, size_t size, _Cmpfun *cmp)
	{	/* sort median of three elements to middle */
	if ((*cmp)(qm, qf) < 0)
		swap(qf, qm, size);
	if ((*cmp)(ql, qm) < 0)
		swap(qm, ql, size);
	if ((*cmp)(qm, qf) < 0)
		swap(qf, qm, size);
	}

static void median(char *qf, char *qm, char *ql, size_t size, _Cmpfun *cmp)
	{	/* sort median element to middle */
	if (40 * size < (size_t)(ql - qf))
		{	/* find median of nine */
		int step = (((ql - qf) / (int)size) / 8 + 1) * size;

		med3(qf, qf + step, qf + 2 * step, size, cmp);
		med3(qm - step, qm, qm + step, size, cmp);
		med3(ql - 2 * step, ql - step, ql, size, cmp);
		med3(qf + step, qm, ql - step, size, cmp);
		}
	else
		med3(qf, qm, ql, size, cmp);
	}

static void adjust_heap(char *qb, size_t h, size_t n,
	size_t size, _Cmpfun *cmp)
	{	/* reheap item at h in heap (char qb[size])[n] */
	size_t h0 = h;
	size_t k = 2 * h + 2;
	char *qh = qb + h * size;
	char *qk = qb + k * size;

	for (; k <= n; k = 2 * k + 2, qk = qb + k * size)
		{	/* percolate hole out to larger/only child */
		if (k == n || (*cmp)(qk, qk - size) < 0)
			--k, qk -= size;
		swap(qh, qk, size);
		h = k, qh = qk;
		}

	for (; h0 < h; )
		{	/* percolate hole back in as far as h0 */
		size_t i = (h - 1) / 2;
		char *qi = qb + i * size;
		if ((*cmp)(qh, qi) <= 0)
			break;
		swap(qi, qh, size);
		h = i, qh = qi;
		}
	}

static void intro_sort(char *qb, size_t n, size_t ideal,
	size_t size, _Cmpfun *cmp)
	{	/* sort recursively */
	for (; 0 < ideal && SORT_MAX < n; )
		{	/* quick sort with fat pivot */
		size_t m = n / 2;
		char *qm = qb + m * size;
		char *qf = qb, *ql = qb + n * size;
		char *pf = qm, *pl = qm + size;
		char *gf, *gl;

		median(qf, qm, ql - size, size, cmp);
		while (qf < pf && (cmp)(pf - size, pf) == 0)
			pf -= size;
		while (pl < ql && (cmp)(pl, pf) == 0)
			pl += size;
		gf = pl;
		gl = pf;

		for (;;)
			{	/* partition */
			int c;

			for (; gf < ql; gf += size)
				if ((c = (cmp)(pf, gf)) < 0)
					;
				else if (0 < c)
					break;
				else
					swap(pl, gf, size), pl += size;
			for (; qf < gl; gl -= size)
				if ((c = (cmp)(gl - size, pf)) < 0)
					;
				else if (0 < c)
					break;
				else
					swap(pf -= size, gl - size, size);
			if (gl == qf && gf == ql)
				break;

			if (gl == qf)
				{	/* reached left end */
				if (pl == gf)
					swap(gf, pf, size);
				else
					rot3(gf, pf, pl, size);
				gf += size, pf += size, pl += size;
				}
			else if (gf == ql)
				{	/* reached right end */
				gl -= size, pl -= size, pf -= size;
				if (gl == pf)
					swap(pf, pl, size);
				else
					rot3(gl, pl, pf, size);
				}
			else
				swap(gf, gl -= size, size), gf += size;
			}

		ideal /= 2, ideal += ideal / 2;
		m = (pf - qb) / (int)size;
		n = (ql - pl) / (int)size;
		if (m <= n)
			intro_sort(qb, m, ideal, size, cmp), qb = pl;
		else
			intro_sort(pl, n, ideal, size, cmp), n = m;
		}

	if (SORT_MAX < n)
		{	/* heap sort */
		size_t h;
		char *qe;

		for (h = n / 2; 0 < h; )
			adjust_heap(qb, --h, n, size, cmp);
		for (qe = qb + n * size; 1 < n; )
			{	/* pop largest item to (shrinking) end */
			swap(qb, qe -= size, size);
			adjust_heap(qb, 0, --n, size, cmp);
			}
		}
	else if (1 < n)
		{	/* insertion sort */
		char *qm;

		for (qm = qb; 0 < --n; )
			{	/* percolate back elements [2, n) */
			qm += size;
			if ((*cmp)(qm, qb) < 0)
				rotate(qb, qm, size);
			else
				{	/* scan backwards for insertion point */
				char *qx = qm, *qx0 = qm;
				for (; (*cmp)(qm, qx0 -= size) < 0; qx = qx0)
					;
				if (qx != qm)
					rotate(qx, qm, size);
				}
			}
		}
	}

void (qsort)(void *base, size_t n, size_t size, _Cmpfun *cmp)
	{	/* sort (char base[size])[n] using introsort */
	intro_sort((char *)base, n, n, size, cmp);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("qsort.c $Rev: 153052 $");
