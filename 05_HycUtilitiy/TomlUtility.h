#pragma once

#include <toml11\toml.hpp>
#include <string>

#include "HycType.h"
#include "StdStringUtility.h"

namespace Hyc
{
    namespace Text
    {
        using TomlNode = toml::value;

        inline bool LoadTomlAndParse(TomlNode& _outRoot,
            const std::string& _path,
            std::string& _outErrorMessage)
        {
            bool result = true;

            try
            {
                _outRoot = toml::parse(_path);
            }
            catch (const std::runtime_error& err)
            {
                _outErrorMessage = err.what();
                result = false;
            }
            catch (const toml::syntax_error& err)
            {
                _outErrorMessage = err.what();
                result = false;
            }
            catch (...)
            {
                _outErrorMessage = "parse failed with unhandled exception";
                result = false;
            }

            return result;
        }

        inline bool LoadTomlAndParse(TomlNode& _outRoot,
            cstring _path,
            std::string& _outErrorMessage)
        {
            bool result = true;

            try
            {
                _outRoot = toml::parse(_path);
            }
            catch (const std::runtime_error& err)
            {
                _outErrorMessage = err.what();
                result = false;
            }
            catch (const toml::syntax_error& err)
            {
                _outErrorMessage = err.what();
                result = false;
            }
            catch (...)
            {
                _outErrorMessage = "parse failed with unhandled exception";
                result = false;
            }

            return result;
        }

        inline bool GetNextTomlNode(const TomlNode& _from, const std::string& _to,
            TomlNode& _outNode)
        {
            bool exist = _from.contains(_to);
            if (!exist)
            {
                return false;
            }

            _outNode = toml::find(_from, _to);
            return true;
        }

        inline bool GetNextTomlNode(const TomlNode& _from, cstring _to,
            TomlNode& _outNode)
        {
            bool exist = _from.contains(_to);
            if (!exist)
            {
                return false;
            }

            _outNode = toml::find(_from, _to);
            return true;
        }

        inline bool GetTomlNode(const TomlNode& _from, const std::string& _to,
            TomlNode& _outNode)
        {
            std::vector<std::string> paths = {};
            Hyc::String::Split(_to, '.', paths);

            TomlNode now = _from;
            TomlNode next = {};

            for (const auto& path : paths)
            {
                if (!GetNextTomlNode(now, path, next))
                {
                    return false;
                }
                else
                {
                    now = next;
                }
            }

            _outNode = next;
            return true;
        }

        inline bool GetTomlNode(const TomlNode& _from, cstring _to,
            TomlNode& _outNode)
        {
            std::vector<std::string> paths = {};
            Hyc::String::Split(_to, '.', paths);

            TomlNode now = _from;
            TomlNode next = {};

            for (const auto& path : paths)
            {
                if (!GetNextTomlNode(now, path, next))
                {
                    return false;
                }
                else
                {
                    now = next;
                }
            }

            _outNode = next;
            return true;
        }

        template <typename T>
        T GetAs(const TomlNode& _node)
        {
            return toml::get<T>(_node);
        }
    }
}
