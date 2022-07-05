/////////////////////////////////////////////////////////////////////////////////////////
//    This file is part of Pongo.
//
//    Copyright (C) 2020 Matthias Hund
//    
//    This program is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public License
//    as published by the Free Software Foundation; either version 2
//    of the License, or (at your option) any later version.
//    
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//    
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef HTTPSTATUS_H
#define HTTPSTATUS_H

enum httpStatus {Continue=100,
SwitchingProtocols=101,
OK=200,
Created=201,
Accepted=202,
NonAuthoritativeInformation=203,
NoContent=204,
ResetContent=205,
PartialContent=206,
MultipleChoices=300,
MovedPermanently=301,
Found=302,
SeeOther=304,
NotModified=305,
UseProxy=306,
TemporaryRedirect=307,
BadRequest=400,
Unauthorized=401,
PaymentRequired=402,
Forbidden=403,
NotFound=404,
MethodNotAllowed=405,
NotAcceptable=406,
ProxyAuthenticationRequired=407,
RequestTimeOut=408,
Conflict=409,
Gone=410,
LengthRequired=411,
PreconditionFailed=412,
RequestEntityTooLarge=413,
RequestURITooLarge=414,
UnsupportedMediaType=415,
RequestedRangeNotSatisfiable=416,
ExpectationFailed=417,
TooManyRequests=429,
InternalServerError=500,
NotImplemented=501,
BadGateway=502,
ServiceUnavailable=503,
GatewayTimeOut=504,
HTTPVersionNotSupported=505};

const int			statusNumber[] 	= {100,
101,
200,
201,
202,
203,
204,
205,
206,
300,
301,
302,
304,
305,
306,
307,
400,
401,
402,
403,
404,
405,
406,
407,
408,
409,
410,
411,
412,
413,
414,
415,
416,
417,
429,
500,
501,
502,
503,
504,
505};

const size_t statusElments = sizeof(statusNumber)/sizeof(int);

const std::string 	statusLabel[] 	= {"Continue",
"Switching Protocols",
"OK",
"Created",
"Accepted",
"Non-Authoritative Information",
"No Content",
"Reset Content",
"Partial Content",
"Multiple Choices",
"Moved Permanently",
"Found",
"See Other",
"Not Modified",
"Use Proxy",
"Temporary Redirect",
"Bad Request",
"Unauthorized",
"Payment Required",
"Forbidden",
"Not Found",
"Method Not Allowed",
"Not Acceptable",
"Proxy Authentication Required",
"Request Time-out",
"Conflict",
"Gone",
"Length Required",
"Precondition Failed",
"Request Entity Too Large",
"Request-URI Too Large",
"Unsupported Media Type",
"Requested range not satisfiable",
"Expectation Failed",
"Too Many Requests"
"Internal Server Error",
"Not Implemented",
"Bad Gateway",
"Service Unavailable",
"Gateway Time-out",
"HTTP Version not supported"};


#endif