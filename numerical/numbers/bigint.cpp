/* KTH ACM Contest Template Library
 *
 * Numeric/Unsigned Big Integer/Simple
 *
 * Features:
 * # read/write in base 10 from/to i-/ostream
 * # cmp/add/sub
 * # multiplication [dependancy: addition]
 * # division and modulo [dependancy for arbitrary denominator: add, sub]
 * # grows dynamically when In Need.
 *
 * What's bad?
 * # not bit-oriented, therefore no bit ops
 *   (e.g. and, not, shift, etc)
 * # no I/O in other bases (easy to implement naively though)
 * # far from the Fastest Thing Possible (TM).
 *
 *
 * Credit:
 *   By
 *        Per Austrin, austrin@kth.se 2002-09-15
 *   Updated by
 *        Per Austrin, Christer Stålstrand, 2002-09-26
 *   Bugfixes (fix in modulo and input/output)
 *        Per Austrin, 2002-10-05
 *   Bugfixes and update (fixes in add and sub, code compression)
 *        Per Austrin, 2003-03-14
 *   Update (the long called-for divmod with arbitrary denominator!!
 *           and more code compression of course)
 *        Per Austrin, 2003-09-22
 *   Update (minor code compression)
 *        Per Austrin, 2004-10-26
 *
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

/* if long longs are disallowed:
 * #define LSIZE 10000
 * #define LIMBDIGS 4
 * typedef int limb;     */
#define LSIZE 1000000000 /* 10^9 */
#define LIMBDIGS 9

typedef long long limb;
typedef vector<limb> bigint;
typedef bigint::const_iterator bcit;
typedef bigint::reverse_iterator brit;
typedef bigint::const_reverse_iterator bcrit;
typedef bigint::iterator bit;

bigint BigInt(limb i) {
  bigint res;
  do res.push_back(i % LSIZE);
  while (i /= LSIZE);
  return res;
}

istream& operator>>(istream& i, bigint& n) {
  string s;  i >> s;
  int l = s.length();
  n.clear();
  while (l > 0) {
    int j = 0;
    for (int k = l > LIMBDIGS ? l-LIMBDIGS: 0; k < l; ++k)
      j = 10*j + s[k]-'0';
    n.push_back(j);
    l -= LIMBDIGS;
  }
  return i;
}

/* Warning: the ostream must be configured to print things
 * with right justification, lest output be ooky */
ostream& operator<<(ostream& o, const bigint& n) {
  int began = 0, ofill = o.fill();
  o.fill('0');
  for (bcrit i = n.rbegin(); i != n.rend(); ++i) {
    if (began) o << setw(LIMBDIGS);
    if (*i) began = 1;
    if (began) o << *i;
  }
  if (!began) o << "0";
  o.fill(ofill);
  return o;
}

/* The base comparison function. semantics like strcmp(...) */
int cmp(const bigint& n1, const bigint& n2) {
  int x = n2.size() - n1.size();
  bcit i = n1.end() - 1, j = n2.end() - 1;
  while (x-- > 0) if (*j--) return -1;
  while (++x < 0) if (*i--) return 1;
  for (; i + 1 != n1.begin(); --i, --j)
    if (*i != *j) return *i-*j;
  return 0;
}

/* The other operators will be automatically defined by STL */
bool operator==(const bigint& n1, const bigint& n2) { 
  return !cmp(n1,n2); }
bool operator<(const bigint& n1, const bigint& n2) { 
  return cmp(n1,n2) < 0; }

bigint& operator+=(bigint& a, const bigint& b) {
  if (a.size() < b.size()) a.resize(b.size());
  limb cy = 0; bit i = a.begin();
  for (bcit j = b.begin(); i != a.end() && 
	 (cy || j < b.end()); ++j, ++i)
    cy += *i + (j < b.end() ? *j : 0), 
      *i = cy % LSIZE, cy /= LSIZE;
  if (cy) a.push_back(cy);
  return a;
}

bool sub(bigint& a, const bigint& b) { /* Ret sign changed */
  if (a.size() < b.size()) a.resize(b.size());
  limb cy = 0; bit i = a.begin();
  for (bcit j = b.begin(); i != a.end() && 
	 (cy || j < b.end()); ++j, ++i) {
    *i -= cy + (j < b.end() ? *j : 0);
    if ((cy = *i < 0)) *i += LSIZE;
  } /* Line below only if sign may change. */
  if (cy) while (i-- > a.begin()) *i = LSIZE - *i;
  return cy;
}

bigint& operator-=(bigint& a, const bigint& b) { 
  sub(a, b); return a; }

bigint& operator*=(bigint& a, limb b) {
  limb cy = 0;
  for (bit i = a.begin(); i != a.end(); ++i)
    cy = cy / LSIZE + *i * b, *i = cy % LSIZE;
  while (cy /= LSIZE) a.push_back(cy % LSIZE);
  return a;
}

bigint& operator*=(bigint& a, const bigint& b) {
  bigint x = a, y, bb = b;
  a.clear();
  for (bcit i = bb.begin(); i != bb.end(); ++i)
    (y = x) *= *i, a += y, x.insert(x.begin(), 0);
  return a;
}

/* a will hold floor(a/b), rest will hold a % b */
bigint& divmod(bigint& a, limb b, limb* rest = NULL) {
  limb cy = 0;
  for (brit i = a.rbegin(); i != a.rend(); ++i)
    cy += *i, *i = cy / b, cy = (cy % b) * LSIZE;
  if (rest) *rest = cy / LSIZE;
  return a;
}

/* returns a, holding a % b, quo will hold floor(a/b).
 * NB!! different semantics from one-limb divmod!!     
 * NB!! quo should be different from a!! */
bigint& divmod(bigint& a, const bigint& b, bigint* quo=NULL) {
  bigint den = b;
  brit j = den.rbegin(), i = a.rbegin();
  for ( ; j != den.rend() && !*j; ++j);
  for ( ; i != a.rend() && !*i; ++i);
  int n = a.rend() - i, m = den.rend() - j;
  if (!m) { /* Division by zero! */ abort(); }
  if (m == 1) {
    bigint q;
    return (quo ? *quo : q) = a, a.resize(1), 
      divmod(quo ? *quo : q, *j, &a.front()), a;
  }
  
  bigint tmp;
  limb den0 = (*++j + *--j * LSIZE) + 1;
  if (quo) quo->clear();
  while (a >= den) { /* Loop invariant: quo * den + a = num */
    limb num0 = (*++i + *--i * LSIZE), z = num0 / den0, cy = 0;
    if (z == 0 && n == m) z = 1;/* Silly degenerate case */
    tmp.resize(n - m - !z);
    if (!z) z = num0 / (*j + 1);/* Non-silly degenerate case*/
    if (quo) tmp.push_back(z), *quo += tmp, tmp.pop_back(); 
    for (bcit j = den.begin(); j != den.end(); ++j)
      cy += *j * z, tmp.push_back(cy % LSIZE), cy /= LSIZE;
    if (cy) tmp.push_back(cy);
    if (tmp.size() > a.size()) tmp.resize(a.size());
    sub(a, tmp);
    while (i != a.rend() && !*i) --n, ++i;
  }
  return a; 
}

bigint& operator/=(bigint& a, const bigint& b) {
  bigint q; return divmod(a, b, &q), a = q; }

bigint& operator%=(bigint& a, const bigint& b) { 
  return divmod(a, b, NULL); }

bigint& operator/=(bigint& a, limb b) { return divmod(a, b); }
limb operator%(const bigint& a, limb b) {
  limb res;
  bigint fubar = a;
  return divmod(fubar, b, &res), res;
}
