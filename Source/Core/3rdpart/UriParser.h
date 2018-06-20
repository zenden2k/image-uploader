#ifndef IU_CORE_3RDPART_URIPARSER_H
#define IU_CORE_3RDPART_URIPARSER_H

#pragma once

#include <string>
#include <uriparser/Uri.h>
#include "Core/Utils/CoreTypes.h"

// Author: Elliot Cameron (3noch) https://github.com/3noch
// http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform

namespace uriparser
{
    class Uri //: boost::noncopyable
    {
        public:
            Uri(std::string uri)
                : uri_(uri)
            {
                UriParserStateA state_;
                state_.uri = &uriParse_;
                isValid_   = uriParseUriA(&state_, uri_.c_str()) == URI_SUCCESS;
            }

            ~Uri() { uriFreeUriMembersA(&uriParse_); }

            bool isValid() const { return isValid_; }

            std::string scheme()   const { return fromRange(uriParse_.scheme); }
            std::string host()     const { return fromRange(uriParse_.hostText); }
            std::string port()     const { return fromRange(uriParse_.portText); }
            std::string path()     const { return fromList(uriParse_.pathHead, "/"); }
            std::string query()    const { return fromRange(uriParse_.query); }
            std::string fragment() const { return fromRange(uriParse_.fragment); }

        private:
            DISALLOW_COPY_AND_ASSIGN(Uri);
            std::string uri_;
            UriUriA     uriParse_;
            bool        isValid_;

            std::string fromRange(const UriTextRangeA & rng) const
            {
                return std::string(rng.first, rng.afterLast);
            }

            std::string fromList(UriPathSegmentA * xs, const std::string & delim) const
            {
                UriPathSegmentStructA * head(xs);
                std::string accum;

                while (head)
                {
                    accum += delim + fromRange(head->text);
                    head = head->next;
                }

                return accum;
            }
    };
}

#endif