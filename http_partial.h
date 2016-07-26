#ifndef HTTP_PARTIAL_H
#define HTTP_PARTIAL_H

#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include "function_base.h"


class Request
{
    public:
        Request(char* raw);
        ~Request();
        std::string getMethod(){return method;}
        std::string getUri(){return uri;}
    private:
        std::string method;
        std::string uri;
        std::string version;
        std::unordered_map<std::string, std::string>* header;
        std::string body;
};


Request::Request(char* raw)
{
    std::string raw_string(raw);
    size_t hb_splitter = raw_string.find("\r\n\r\n"); // Header-body splitter
    body = raw_string.substr(hb_splitter + 1, -1);
    raw_string = raw_string.substr(0, hb_splitter); // The rest is header part
    ::replaceAll(raw_string, "\r\n", "\n");
    std::vector<std::string> ss = ::split(raw_string, '\n'); // Each line is a field of header
    // First line = method + " " + uri + " " + http version
    std::vector<std::string> first_line = ::split(ss[0], ' ');
    method = first_line[0];
    uri = first_line[1];
    version = first_line[2];
    header = new std::unordered_map<std::string, std::string>();
    for (size_t i = 1; i < ss.size() - 1; i++)
    {
        size_t pos = ss[i].find(":");
        // The rest of the line = field name + ": " + value
        header->insert(std::make_pair(ss[i].substr(0, pos), ss[i].substr(pos + 2, -1)));
    }
}


Request::~Request()
{
    header->clear();
    delete header;
}


class Response
{
    public:
        Response(int code, std::string phrase, std::string html);
        ~Response();
        std::string toString();
        void add_header(std::unordered_map<std::string, std::string>* extra_header);

    private:
        std::string version;
        int status_code;
        std::string reason_phrase;
        std::unordered_map<std::string, std::string>* header; // Dictionary storage
        std::string body;
};


Response::Response(int code, std::string phrase, std::string html)
{
    version = "HTTP/1.1";
    status_code = code;
    body = html;
    reason_phrase = phrase;
    header = new std::unordered_map<std::string, std::string>();
    header->insert(std::make_pair("Cache-Control", "max-age=0"));
    header->insert(std::make_pair("Content-Type", "text/html"));
    // Converting size_t to string
    std::stringstream ss;
    ss << html.length();
    header->insert(std::make_pair("Content-Length", ss.str()));
}


Response::~Response()
{
    body = "";
    header->clear();
    delete header;
}


// Convert to string for easier output
std::string Response::toString()
{
    std::string first = version + " " + std::to_string(status_code) + " " + reason_phrase + "\r\n";
    std::string header_s = "";
    for (auto i = header->begin(); i != header->end(); ++i)
        header_s = header_s + i->first + ": " + i->second + "\r\n";
    std::string finale = first + header_s + "\r\n" + body;
    return finale;
}


// Add extra header
void Response::add_header(std::unordered_map<std::string, std::string>* extra_header)
{
    header->insert(extra_header->begin(), extra_header->end());
}
#endif
