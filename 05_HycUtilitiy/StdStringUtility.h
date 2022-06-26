#pragma once

#include <string>
#include <vector>

#include "HycType.h"

namespace Hyc
{
    namespace String
    {
        inline void Split(const std::string& _source, char _symbol,
            std::vector<std::string>& _outResult)
        {
            _outResult.clear();
            std::string::size_type pos1 = 0;
            std::string::size_type pos2 = _source.find(_symbol);
            while (std::string::npos != pos2)
            {
                _outResult.push_back(_source.substr(pos1, pos2 - pos1));

                pos1 = pos2 + 1;
                pos2 = _source.find(_symbol, pos1);
            }
            if (pos1 != _source.length())
            {
                _outResult.push_back(_source.substr(pos1));
            }
        }

        inline void Split(cstring _source, char _symbol,
            std::vector<std::string>& _outResult)
        {
            _outResult.clear();
            std::string source(_source);
            std::string::size_type pos1 = 0;
            std::string::size_type pos2 = source.find(_symbol);
            while (std::string::npos != pos2)
            {
                _outResult.push_back(source.substr(pos1, pos2 - pos1));

                pos1 = pos2 + 1;
                pos2 = source.find(_symbol, pos1);
            }
            if (pos1 != source.length())
            {
                _outResult.push_back(source.substr(pos1));
            }
        }

        inline void Split(const std::string& _source, const std::string& _symbol,
            std::vector<std::string>& _outResult)
        {
            _outResult.clear();
            std::string::size_type pos1 = 0;
            std::string::size_type pos2 = _source.find(_symbol);
            while (std::string::npos != pos2)
            {
                _outResult.push_back(_source.substr(pos1, pos2 - pos1));

                pos1 = pos2 + _symbol.size();
                pos2 = _source.find(_symbol, pos1);
            }
            if (pos1 != _source.length())
            {
                _outResult.push_back(_source.substr(pos1));
            }
        }

        inline void Split(cstring _source, cstring _symbol,
            std::vector<std::string>& _outResult)
        {
            _outResult.clear();
            std::string source(_source);
            std::string symbol(_symbol);
            std::string::size_type pos1 = 0;
            std::string::size_type pos2 = source.find(_symbol);
            while (std::string::npos != pos2)
            {
                _outResult.push_back(source.substr(pos1, pos2 - pos1));

                pos1 = pos2 + symbol.size();
                pos2 = source.find(symbol, pos1);
            }
            if (pos1 != source.length())
            {
                _outResult.push_back(source.substr(pos1));
            }
        }
    }
}
