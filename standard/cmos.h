#define from_bcd(val)  ((val / 16) * 10 + (val & 0xf))

int16_t timezone = 3;
uint16_t century = 2;

struct timeval {
	uint32_t tv_sec;
	uint32_t tv_usec;
};

typedef struct date {
	uint16_t h, m, s, d, mo, y;
} date_t;

void cmos_dump(uint16_t * values) {
	uint16_t index;
	for (index = 0; index < 128; ++index) {
		write_port(0x70, index);
		values[index] = read_port(0x71);
	}
}

/**
 * Get the current month and day.
 *
 * @param month Pointer to a short to store the month
 * @param day   Pointer to a short to store the day
 */
void get_date(uint16_t * month, uint16_t * day, uint16_t * year) {
	uint16_t values[128]; /* CMOS dump */
	cmos_dump(values);

	*month = from_bcd(values[8]);
	*day   = from_bcd(values[7]);
	*year  = (century*1000) + from_bcd(values[9]);
}

/**
 * Get the current time.
 *
 * @param hours   Pointer to a short to store the current hour (/24)
 * @param minutes Pointer to a short to store the current minute
 * @param seconds Pointer to a short to store the current second
 */
bool get_time(uint16_t * hours, uint16_t * minutes, uint16_t * seconds) {
	bool ovf = false;
	uint16_t values[128]; /* CMOS dump */
	cmos_dump(values);

	*hours   = from_bcd(values[4]);
	*hours   += timezone;
	if (*hours >= 24) {
		*hours = 24 - *hours;
		ovf = true;
	}
	*minutes = from_bcd(values[2]);
	*seconds = from_bcd(values[0]);
	return ovf;
}

uint32_t secs_of_years(int years) {
	uint32_t days = 0;
	years += 2000;
	while (years > 1969) {
		days += 365;
		if (years % 4 == 0) {
			if (years % 100 == 0) {
				if (years % 400 == 0) {
					days++;
				}
			} else {
				days++;
			}
		}
		years--;
	}
	return days * 86400;
}

bool is_leap_year(uint16_t year) {
	if (year % 4 == 0) {
		if (year % 100 == 0) {
			if (year % 400 == 0)
				return true;
			else
                		return false;
        	}
        	else
            		return true;
    	}
    	else
        	return false;
}

uint32_t secs_of_month(int months, int year) {
	year += 2000;
	uint32_t days = 0;
	switch(months) {
		case 11:
			days += 30;
		case 10:
			days += 31;
		case 9:
			days += 30;
		case 8:
			days += 31;
		case 7:
			days += 31;
		case 6:
			days += 30;
		case 5:
			days += 31;
		case 4:
			days += 30;
		case 3:
			days += 31;
		case 2:
			days += 28;
			if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) {
				days++;
			}
		case 1:
			days += 31;
		default:
			break;
	}
	return days * 86400;
}

int gettimeofday(struct timeval * t, void *z) {
	uint16_t values[128];
	cmos_dump(values);

	/* Math Time */
	uint32_t time = secs_of_years(from_bcd(values[9]) - 1) +
					secs_of_month(from_bcd(values[8]) - 1, from_bcd(values[9])) + 
					(from_bcd(values[7]) - 1) * 86400 +
					(from_bcd(values[4])) * 3600 +
					(from_bcd(values[2])) * 60 +
					from_bcd(values[0]) +
					0;
	t->tv_sec = time;
	t->tv_usec = 0;
	return 0;
}

uint32_t now() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec;
}

void get_today(date_t *dt) {
	get_date(&dt->mo, &dt->d, &dt->y);
	if (get_time(&dt->h, &dt->m, &dt->s))
		dt->d++;
	if (dt->d > 31 && dt->mo % 2 != 0 && dt->mo != 9) {
		dt->d = 1;
		dt->mo++;
	}
	if (dt->d > 30 && dt->mo % 2 == 0 && dt->mo != 2) {
		dt->d = 1;
		dt->mo++;
	}
	if (dt->mo == 2) {
		bool leap = is_leap_year(dt->y);
		if (leap && dt->d > 29) {
			dt->mo++;
			dt->d=1;
		}
		if (!leap && dt->d > 28) {
			dt->mo++;
			dt->d=1;
		}
	}
	if (dt->mo > 12)
		dt->y++;
		
}

date_t ttime;
void today() {
	get_today(&ttime);
	printf("[%u/%u/%u] [%u:%u:%u]",  ttime.d, ttime.mo, ttime.y, ttime.h, ttime.m, ttime.s);
}
