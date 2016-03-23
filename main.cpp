#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

const int base = 300000;
const int fine = 1500000;
const int timeSeat = 10 * 60;
const int maxTimeWork = 9 * 3600;
const int maxAddTrip = 3600;
const double INF = 1000000000000000;

int getCostLen(int l)
{
	return l;
}

double getCostTime(int t)
{
	return ((double)t * base / (12 * 3600)) / 2;
}

int a, g, q, d;
int n;

pair<int, int> points[2000];

struct Order
{
	int from, to, moment;
	int id;
};

bool cmp(Order a, Order b)
{
	return (a.moment < b.moment);
}

Order orders[2000];

struct Driver
{
	int garage, start, finish;
	int currTime, point;
	int workTime;
	int currOrder;
};

Driver drivers[2000];

int dist[2000][2000];
int time[2000][2000];

int listDrivers[2009][2009];
int freeDrivers[2000];

enum OrderComplete
{
	none, push, firstSecond, secondFirst
};

struct Ans
{
	bool correct;
	bool firstOrder;
	int addTime;
	int addLen;
	OrderComplete orderComplete;
};

enum TypeComand
{
	go, pick, drop
};

struct Comand
{
	TypeComand tp;
	int moment, key1, key2;
};

vector<Comand> driverComands[2000];

void move1(int d, int moment, int v)
{
	if (drivers[d].point != v)
	{
		Comand c;
		c.tp = go;
		c.moment = moment;
		c.key1 = v;
		c.key2 = -1;
		driverComands[d].push_back(c);
		drivers[d].point = v;
	}
}

void pick1(int d, int moment, int idOrder)
{
	Comand c;
	c.tp = pick;
	c.moment = moment;
	c.key1 = orders[idOrder].id;
	c.key2 = -1;
	driverComands[d].push_back(c);
}

void pick2(int d, int moment, int idOrder1, int idOrder2)
{
	Comand c;
	c.tp = pick;
	c.moment = moment;
	c.key1 = orders[idOrder1].id;
	c.key2 = orders[idOrder2].id;
	driverComands[d].push_back(c);
}

void drop1(int d, int moment, int idOrder)
{
	Comand c;
	c.tp = drop;
	c.moment = moment;
	c.key1 = orders[idOrder].id;
	c.key2 = -1;
	driverComands[d].push_back(c);
}

void drop2(int d, int moment, int idOrder1, int idOrder2)
{
	Comand c;
	c.tp = drop;
	c.moment = moment;
	c.key1 = orders[idOrder1].id;
	c.key2 = orders[idOrder2].id;
	driverComands[d].push_back(c);
}

void pushOrder(int d)
{
	Driver dr = drivers[d];
	Order curr = orders[drivers[d].currOrder];

	if (curr.from < a)
	{
		int currt = dr.currTime;
		move1(d, currt, curr.from);

		currt = curr.moment;

		pick1(d, currt, dr.currOrder);
		currt += timeSeat;

		move1(d, currt, curr.to);
		currt += time[curr.from][curr.to];

		drop1(d, currt, dr.currOrder);
		currt += timeSeat;

		drivers[d].currTime = currt;
		drivers[d].workTime -= (time[dr.point][curr.from] + time[curr.from][curr.to]);
		drivers[d].currOrder = -1;
	}
	else
	{
		int maxStart = curr.moment - (time[curr.from][curr.to] + 2 * timeSeat);
		int minStart = maxStart - maxAddTrip;
		int realStart = max(minStart, dr.currTime + time[dr.point][curr.from]);

		int currt = dr.currTime;

		move1(d, currt, curr.from);
		currt = realStart;

		pick1(d, currt, dr.currOrder);
		currt += timeSeat;

		move1(d, currt, curr.to);
		currt += time[curr.from][curr.to];

		drop1(d, currt, dr.currOrder);
		currt += timeSeat;

		drivers[d].currTime = currt;
		drivers[d].workTime -= (time[dr.point][curr.from] + time[curr.from][curr.to]);
		drivers[d].currOrder = -1;
	}
}

void pushOrder1(int d)
{
	Driver dr = drivers[d];
	Order curr = orders[drivers[d].currOrder];

	if (curr.from < a)
	{
		int currt = dr.currTime;

		currt = curr.moment;

		currt += timeSeat;

		currt += time[curr.from][curr.to];

		currt += timeSeat;

		drivers[d].point = curr.to;
		drivers[d].currTime = currt;
		drivers[d].workTime -= (time[dr.point][curr.from] + time[curr.from][curr.to]);
		drivers[d].currOrder = -1;
	}
	else
	{
		int maxStart = curr.moment - (time[curr.from][curr.to] + 2 * timeSeat);
		int minStart = maxStart - maxAddTrip;
		int realStart = max(minStart, dr.currTime + time[dr.point][curr.from]);

		int currt = dr.currTime;

		currt = realStart;

		currt += timeSeat;

		currt += time[curr.from][curr.to];

		currt += timeSeat;

		drivers[d].point = curr.to;
		drivers[d].currTime = currt;
		drivers[d].workTime -= (time[dr.point][curr.from] + time[curr.from][curr.to]);
		drivers[d].currOrder = -1;
	}
}

Ans getAns(int d, int idOrder)
{
	Ans ans;

	int vFrom = orders[idOrder].from;
	int vTo = orders[idOrder].to;
	int moment = orders[idOrder].moment;
	Driver d1 = drivers[d];

	enum { inCity, outCity };

	int dir = (vFrom < a ? inCity : outCity);

	if (dir == inCity)
	{
		if (d1.currOrder == -1)
		{
			ans.orderComplete = none;

			if (d1.workTime < time[d1.point][vFrom] + time[vFrom][vTo] + time[vTo][d1.garage])
			{
				ans.correct = false;
				return ans;
			}

			if (d1.currTime + time[d1.point][vFrom] > moment)
			{
				ans.correct = false;
				return ans;
			}

			if (moment + time[vFrom][vTo] + 2 * timeSeat + time[vTo][d1.garage] > d1.finish)
			{
				ans.correct = false;
				return ans;
			}

			ans.correct = true;
			if (d1.currTime == d1.start)
				ans.firstOrder = true;
			else
				ans.firstOrder = false;

			ans.addTime = moment - d1.currTime + time[vFrom][vTo] + 2 * timeSeat;
			ans.addLen = dist[d1.point][vFrom] + dist[vFrom][vTo];

			return ans;
		}
		else
		{
			int curr1 = d1.currOrder;

			if (orders[curr1].moment != moment)
			{
				pushOrder1(d);
				ans = getAns(d, idOrder);
				ans.orderComplete = push;
				drivers[d] = d1;
				return ans;
			}

			if (orders[curr1].from != vFrom)
			{
				ans.correct = false;
				return ans;
			}

			if (d1.currTime + time[d1.point][vFrom] > moment)
			{
				ans.correct = false;
				return ans;
			}

			int to1 = orders[curr1].to;
			int to2 = vTo;

			int base1 = time[vFrom][to1] + 2 * timeSeat;
			int base2 = time[vFrom][to2] + 2 * timeSeat;

			int time1, len1, time2, len2;

			int sumT1 = time[vFrom][to1] + time[to1][to2] + 3 * timeSeat;

			if (moment + sumT1 + time[to2][d1.garage] <= d1.finish &&
				time[d1.point][vFrom] + time[vFrom][to1] + time[to1][to2] + time[to2][d1.garage] <= d1.workTime &&
				base2 + maxAddTrip >= sumT1)
			{
				time1 = sumT1 + moment - d1.currTime;
				len1 = dist[d1.point][vFrom] + dist[vFrom][to1] + dist[to1][to2];
			}
			else
			{
				time1 = -1;
			}


			int sumT2 = time[vFrom][to2] + time[to2][to1] + 3 * timeSeat;

			if (moment + sumT2 + time[to1][d1.garage] <= d1.finish &&
				time[d1.point][vFrom] + time[vFrom][to2] + time[to2][to1] + time[to1][d1.garage] <= d1.workTime &&
				base1 + maxAddTrip >= sumT2)
			{
				time2 = sumT2 + moment - d1.currTime;
				len2 = dist[d1.point][vFrom] + dist[vFrom][to2] + dist[to2][to1];
			}
			else
			{
				time2 = -1;
			}

			int bestTime, bestLen;

			if (time1 == -1 && time2 == -1)
			{
				ans.correct = false;
				return ans;
			}


			ans.correct = true;

			if (time1 == -1)
			{
				bestTime = time2;
				bestLen = len2;
				ans.orderComplete = secondFirst;
			}
			else if (time2 == -1)
			{
				bestTime = time1;
				bestLen = len1;
				ans.orderComplete = firstSecond;
			}
			else
			{
				double c1 = getCostLen(len1) + getCostTime(time1);
				double c2 = getCostLen(len2) + getCostTime(time2);

				if (c1 < c2)
				{
					bestTime = time1;
					bestLen = len1;
					ans.orderComplete = firstSecond;
				}
				else
				{
					bestTime = time2;
					bestLen = len2;
					ans.orderComplete = secondFirst;
				}
			}

			ans.firstOrder = false;
			ans.addTime = bestTime - (time[vFrom][to1] + 2 * timeSeat);
			ans.addLen = bestLen - dist[vFrom][to1];

			return ans;
		}
	}
	else
	{
		if (d1.currOrder == -1)
		{
			ans.orderComplete = none;

			int baseTime = time[vFrom][vTo] + 2 * timeSeat;
			int minStart = moment - baseTime - maxAddTrip;
			int maxStart = moment - baseTime;

			if (d1.currTime + time[d1.point][vFrom] > maxStart)
			{
				ans.correct = false;
				return ans;
			}

			int realStart = max(minStart, d1.currTime + time[d1.point][vFrom]);
			if (realStart + baseTime + time[vTo][d1.garage] > d1.finish)
			{
				ans.correct = false;
				return ans;
			}

			if (time[d1.point][vFrom] + time[vFrom][vTo] + time[vTo][d1.garage] > d1.workTime)
			{
				ans.correct = false;
				return ans;
			}

			ans.correct = true;
			if (d1.currTime == d1.start)
				ans.firstOrder = true;
			else
				ans.firstOrder = false;

			ans.addLen = dist[d1.point][vFrom] + dist[vFrom][vTo];
			ans.addTime = time[d1.point][vFrom] + time[vFrom][vTo] + 2 * timeSeat;

			return ans;
		}
		else
		{
			int curr1 = d1.currOrder;
			if (vTo != orders[curr1].to)
			{
				pushOrder1(d);
				ans = getAns(d, idOrder);
				ans.orderComplete = push;
				drivers[d] = d1;
				return ans;
			}

			int from1 = orders[curr1].from;
			int from2 = vFrom;
			int moment1 = orders[curr1].moment;
			int moment2 = moment;

			int baseTime1 = time[from1][vTo] + 2 * timeSeat;
			int baseTime2 = time[from2][vTo] + 2 * timeSeat;


			bool variant1 = true;
			int len1 = dist[d1.point][from1] + dist[from1][from2] + dist[from2][vTo];
			int timeArrived = min(moment2, moment1);

			int minStart2, maxStart2;
			maxStart2 = timeArrived - baseTime2;

			int minStart1, maxStart1;
			minStart1 = max(moment2 - baseTime2 - maxAddTrip - time[from1][from2] - timeSeat, moment1 - baseTime1 - maxAddTrip);
			maxStart1 = maxStart2 - time[from1][from2] - timeSeat;

			if (minStart1 > maxStart1)
				variant1 = false;

			if (d1.currTime + time[d1.point][from1] > maxStart1)
				variant1 = false;

			int realStart1 = max(minStart1, d1.currTime + time[d1.point][from1]);

			if (realStart1 + time[from1][from2] + time[from2][vTo] + 3 * timeSeat + time[vTo][d1.garage] > d1.finish)
				variant1 = false;

			if (time[d1.point][from1] + time[from1][from2] + time[from2][vTo] + time[vTo][d1.garage] > d1.workTime)
				variant1 = false;

			if (moment1 - realStart1 > baseTime1 + maxAddTrip)
				variant1 = false;

			int time1 = -1;

			if (variant1)
			{
				time1 = realStart1 - d1.currTime + time[d1.point][from1] + time[from1][from2] + time[from2][vTo] + 3 * timeSeat;
			}



			bool variant2 = true;
			int len2 = dist[d1.point][from2] + dist[from2][from1] + dist[from1][vTo];

			maxStart1 = timeArrived - baseTime1;

			minStart2 = max(moment1 - baseTime1 - maxAddTrip - time[from2][from1] - timeSeat, moment2 - baseTime2 - maxAddTrip);
			maxStart2 = maxStart1 - time[from2][from1] - timeSeat;

			if (minStart2 > maxStart2)
				variant2 = false;

			if (d1.currTime + time[d1.point][from2] > maxStart2)
				variant2 = false;

			int realStart2 = max(minStart2, d1.currTime + time[d1.point][from2]);

			if (realStart2 + time[from2][from1] + time[from1][vTo] + 3 * timeSeat + time[vTo][d1.garage] > d1.finish)
				variant2 = false;

			if (time[d1.point][from2] + time[from2][from1] + time[from1][vTo] + time[vTo][d1.garage] > d1.workTime)
				variant2 = false;

			if (moment2 - realStart2 > baseTime2 + maxAddTrip)
				variant2 = false;

			int time2 = -1;

			if (variant2)
			{
				time2 = realStart2 - d1.currTime + time[d1.point][from2] + time[from2][from1] + time[from1][vTo] + 3 * timeSeat;
			}



			if (!variant1 && !variant2)
			{
				pushOrder1(d);
				ans = getAns(d, idOrder);
				ans.orderComplete = push;
				drivers[d] = d1;
				return ans;
			}

			ans.correct = true;
			ans.firstOrder = false;

			int bestTime, bestLen;

			if (!variant1)
			{
				bestTime = time2;
				bestLen = len2;
				ans.orderComplete = secondFirst;
			}
			else if (!variant2)
			{
				bestTime = time1;
				bestLen = len1;
				ans.orderComplete = firstSecond;
			}
			else
			{
				double cost1 = getCostLen(len1) + getCostTime(time1);
				double cost2 = getCostLen(len2) + getCostTime(time2);

				if (cost1 < cost2)
				{
					bestLen = len1;
					bestTime = time1;
					ans.orderComplete = firstSecond;
				}
				else
				{
					bestLen = len2;
					bestTime = time2;
					ans.orderComplete = secondFirst;
				}
			}

			ans.addTime = bestTime - (time[from1][vTo] + 2 * timeSeat);
			ans.addLen = bestLen - dist[from1][vTo];

			return ans;
		}
	}
}

void push2Order(int d, int idOrder2)
{
	Ans ans = getAns(d, idOrder2);

	if (ans.orderComplete == push)
	{
		pushOrder(d);
		drivers[d].currOrder = idOrder2;
	}
	else
	{
		Order ord1 = orders[drivers[d].currOrder];
		Order ord2 = orders[idOrder2];
		int id1 = drivers[d].currOrder;
		int id2 = idOrder2;
		Driver dr = drivers[d];

		if (ans.orderComplete == secondFirst)
		{
			swap(ord1, ord2);
			swap(id1, id2);
		}

		if (ord1.from < a)
		{
			int currt = dr.currTime;

			move1(d, currt, ord1.from);
			currt += time[dr.point][ord1.from];
			currt = ord1.moment;

			pick2(d, ord1.moment, id1, id2);
			currt += timeSeat;

			move1(d, currt, ord1.to);
			currt += time[ord1.from][ord1.to];

			drop1(d, currt, id1);
			currt += timeSeat;

			move1(d, currt, ord2.to);
			currt += time[ord1.to][ord2.to];

			drop1(d, currt, id2);
			currt += timeSeat;

			drivers[d].currOrder = -1;
			drivers[d].currTime = currt;
			drivers[d].workTime -= (time[dr.point][ord1.from] + time[ord1.from][ord1.to] + time[ord1.to][ord2.to]);
		}
		else
		{
			int baseTime1 = time[ord1.from][ord1.to] + 2 * timeSeat;
			int baseTime2 = time[ord2.from][ord2.to] + 2 * timeSeat;
			int maxTime1 = baseTime1 + maxAddTrip;
			int maxTime2 = baseTime2 + maxAddTrip;

			int minStart = max(ord2.moment - maxTime2 - time[ord1.from][ord2.from] - timeSeat,
				ord1.moment - maxTime1);

			int currt = dr.currTime;

			move1(d, currt, ord1.from);
			currt += time[dr.point][ord1.from];

			int realStart = max(currt, minStart);
			currt = realStart;

			pick1(d, currt, id1);
			currt += timeSeat;

			move1(d, currt, ord2.from);
			currt += time[ord1.from][ord2.from];

			pick1(d, currt, id2);
			currt += timeSeat;

			move1(d, currt, ord2.to);
			currt += time[ord2.from][ord2.to];

			drop2(d, currt, id1, id2);
			currt += timeSeat;

			drivers[d].currOrder = -1;
			drivers[d].currTime = currt;
			drivers[d].workTime -= (time[dr.point][ord1.from] + time[ord1.from][ord2.from] + time[ord2.from][ord2.to]);
		}
	}
}

int idOrderSort;

double getCostAns(Ans &ans, int idOrder)
{
	if (!ans.correct)
		return fine;
	return getCostLen(ans.addLen) + getCostTime(ans.addTime) + (double)base * ans.firstOrder / (5 - 4.0 * idOrder / (q - 1));
}

bool cmpDrivers(int d1, int d2)
{
	int idOrder = idOrderSort;
	Ans ans1 = getAns(d1, idOrder);
	Ans ans2 = getAns(d2, idOrder);
	double c1 = getCostAns(ans1, idOrder);
	double c2 = getCostAns(ans2, idOrder);
	if (c1 != c2)
		return c1 < c2;
	return (d1 < d2);
}

int getDriver(int id)
{
	idOrderSort = id;
	sort(listDrivers[id], listDrivers[id] + d, cmpDrivers);
	int bestDriver = listDrivers[id][0];
	Ans ans = getAns(bestDriver, id);
	double bestCost = getCostAns(ans, id);
	int bestId = -1;

	for (int i = 0; i < id; ++i)
	{
		int currDriver = listDrivers[i][0];
		if (drivers[currDriver].currOrder == i && currDriver != listDrivers[id][0])
		{
			int i1 = freeDrivers[i];
			while (i1 < d && !getAns(listDrivers[i][i1], i).correct)
				++i1;
			freeDrivers[i] = i1;
			if (i1 == d)
				continue;

			int driver2 = listDrivers[i][i1];
			drivers[currDriver].currOrder = -1;
			Ans ansp = getAns(currDriver, i);
			Ans ans1 = getAns(currDriver, id);
			Ans ans2 = getAns(driver2, i);
			drivers[currDriver].currOrder = i;
			if (bestCost > getCostAns(ans1, id) + getCostAns(ans2, id) - getCostAns(ansp, id))
			{
				bestCost = getCostAns(ans1, id) + getCostAns(ans2, id) - getCostAns(ansp, id);
				bestId = i;
			}
		}
	}

	if (bestId == -1)
		return listDrivers[id][0];

	int currDriver = listDrivers[bestId][0];
	drivers[currDriver].currOrder = -1;
	int driver2 = listDrivers[bestId][freeDrivers[bestId]];
	Ans ans2;
	ans2.correct = false;
	if (driver2 != -1)
	{
		ans2 = getAns(driver2, bestId);
		swap(listDrivers[bestId][0], listDrivers[bestId][freeDrivers[bestId]]);
	}
	if (ans2.correct)
	{
		if (drivers[driver2].currOrder == -1)
			drivers[driver2].currOrder = bestId;
		else
			push2Order(driver2, bestId);
	}

	for (int i = 0; i < d; ++i)
		if (listDrivers[id][i] == currDriver)
		{
			for (int j = i - 1; j >= 0; --j)
				swap(listDrivers[id][j + 1], listDrivers[id][j]);
			break;
		}
	return currDriver;
}

void moveToGarage(int d)
{
	int p = drivers[d].point;
	move1(d, drivers[d].currTime, drivers[d].garage);
	drivers[d].workTime -= time[p][drivers[d].garage];
}

int main()
{
	//freopen("input.txt", "r", stdin);
	//freopen("output.txt", "w", stdout);

	long long seed;

	cin >> seed;
	cin >> a >> g >> q >> d;

	n = a + g + q;

	for (int i = 0; i < n; ++i)
		cin >> points[i].first >> points[i].second;

	for (int i = 0; i < q; ++i)
	{
		cin >> orders[i].from >> orders[i].to >> orders[i].moment;
		orders[i].id = i;
	}

	for (int i = 0; i < d; ++i)
	{
		cin >> drivers[i].garage >> drivers[i].start >> drivers[i].finish;
		drivers[i].currTime = drivers[i].start;
		drivers[i].point = drivers[i].garage;
		drivers[i].workTime = maxTimeWork;
		drivers[i].currOrder = -1;
	}

	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			cin >> dist[i][j];

	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			cin >> time[i][j];

	sort(orders, orders + q, cmp);

	for (int i = 0; i < q; ++i)
	{
		for (int j = 0; j < d; ++j)
			listDrivers[i][j] = j;
		listDrivers[i][d] = -1;
		freeDrivers[i] = 1;
	}

	int cntMissed = 0;

	for (int i = 0; i < q; ++i)
	{
		int idDriver = getDriver(i);
		Ans ans = getAns(idDriver, i);

		if (ans.correct)
		{
			if (drivers[idDriver].currOrder == -1)
			{
				drivers[idDriver].currOrder = i;
			}
			else
			{
				push2Order(idDriver, i);
			}
		}
		else
		{
			cntMissed++;
		}
	}

	for (int i = 0; i < d; ++i)
		if (drivers[i].currOrder != -1)
			pushOrder(i);

	for (int i = 0; i < d; ++i)
	{
		moveToGarage(i);
	}

	cout << seed << endl;

	for (int i = 0; i < d; ++i)
	{
		printf("driver %d:", i);

		if (driverComands[i].size())
			printf(" yes\n");
		else
			printf(" no\n");

		//if (drivers[i].workTime < 0)
		//printf("ERROR\n");

		for (int j = 0; j < (int)driverComands[i].size(); ++j)
		{
			Comand c = driverComands[i][j];

			if (c.tp == go)
			{
				printf("move %d %d\n", c.moment, c.key1);
			}
			else
			{
				if (c.tp == pick)
				{
					printf("pick");
				}
				else
				{
					printf("drop");
				}

				if (c.key2 != -1)
					printf("2 %d %d %d\n", c.moment, c.key1, c.key2);
				else
					printf("1 %d %d\n", c.moment, c.key1);
			}
		}

		if (driverComands[i].size())
		{
			printf("end\n");
		}
	}
}