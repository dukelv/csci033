/*********************************************************
 * config.h - Configuration data for the driver.c program.
 *********************************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_

/* 
 * CPEs for the baseline (naive) version of the rotate function that
 * was handed out to the students. Rd is the measured CPE for a dxd
 * image. Run the driver.c program on your system to get these
 * numbers.  
 */
#define R64    18.7
#define R128   17.9
#define R256   18.6
#define R512   21.0
#define R1024  33.0

/* 
 * CPEs for the baseline (naive) version of the invert function that
 * was handed out to the students. Id is the measure CPE for a dxd
 * image. Run the driver.c program on your system to get these
 * numbers.  
 */
#define I32   57.8
#define I64   58.0
#define I128  66.2
#define I256  68.5
#define I512  77.6

/* 
 * CPEs for the baseline (naive) version of the reverse function that
 * was handed out to the students. Vd is the measure CPE for a dxd
 * image. Run the driver.c program on your system to get these
 * numbers.  
 */
#define V800   8.6 
#define V5000  9.8
#define V10000  10.2
#define V15000  10.3
#define V30000 10.4


#endif /* _CONFIG_H_ */
