/*
 * Version.hpp
 *
 *  Created on: 15 Mar 2018
 *      Author: David
 */

#ifndef SRC_VERSION_HPP_
#define SRC_VERSION_HPP_

#define VERSION_TEXT_MAIN		"3.4.1-pre2"

#ifdef SUPPORT_ENCODER
#define VERSION_TEXT	VERSION_TEXT_MAIN "+enc"
#else
#define VERSION_TEXT	VERSION_TEXT_MAIN
#endif

#endif /* SRC_VERSION_HPP_ */
