/*
 * server_db.h
 *
 *  Created on: May 18, 2017
 *      Author: root
 */

#ifndef SERVER_DB_H_
#define SERVER_DB_H_

#include "antlib.h"

void init_mydb(void* config,int no);

mydb* get_mydb(int no);


#endif /* SERVER_DB_H_ */
