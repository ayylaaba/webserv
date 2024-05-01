#!/usr/bin/env python3

import http.cookies 
 
# Create a cookie 
cookie = http.cookies.SimpleCookie() 
cookie["cookie_name"] = "cookie_value" 
cookie["cookie_name"]["expires"] = "Wed, 01-Feb-2023 12:00:00 GMT" 
 
# Set the cookie in the header of a response 
print("Set-Cookie: " + cookie.output(header="").strip()) 

# import cgi
# # while 1:
# #     a = "n"
# print("Content-type: text/html\n")
# print("<html><head><title>CGI POST Test</title></head><body>")
# print("<h1>POST Data:</h1>")

# form = cgi.FieldStorage()
# for key in form.keys():
#     print(f"<p>{key}: {form[key].value}</p>")

# print("</body></html>")