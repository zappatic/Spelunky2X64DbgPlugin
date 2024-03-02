#pragma once

#include "pluginmain.h"
#include <cstdint>
#include <utility>

namespace S2Plugin
{
    // For the proper use you need to use template to define the types
    // otherwise the functions ::find ::at ::contains ::node::key ::node::value, won't work properly
    // when not using template, you need to define the size of key and value with the correct constructor
    // this can also be used as std::set, just set the value size to 0
    // it's not the safes implementation but the one that was needed

    template <class Key = size_t, class Value = size_t>
    struct StdMap
    {
        // only for the template
        StdMap(size_t addr) : address(addr)
        {
            keytype_size = sizeof(Key);
            valuetype_size = sizeof(Value);
            set_offsets();
        };

        // value size only needed for value() function
        StdMap(size_t addr, uint8_t keyAlignment, uint8_t valueAlignment, size_t keySize) : address(addr)
        {
            keytype_size = keySize;
            valuetype_size = sizeof(Value);
            set_offsets(keyAlignment, valueAlignment);
        };
        StdMap(size_t addr, uint8_t keyAlignment, uint8_t valueAlignment, size_t keySize, size_t valueSize) : address(addr)
        {
            keytype_size = keySize;
            valuetype_size = valueSize;
            set_offsets(keyAlignment, valueAlignment);
        };

        struct Node
        {
            Node(size_t addr, const StdMap<Key, Value>* _map) : node_ptr(addr), parent_map(_map){};
            Node(Node& t) : node_ptr(t.node_ptr), parent_map(t.parent_map){};
            Key key() const
            {
                auto offset = key_ptr();
                // would probably be better with Read function but
                // probably doesn't matter as for large structures you will just grab the address most of the time
                switch (parent_map->keytype_size)
                {
                    case size_byte:
                        return (Key)Script::Memory::ReadByte(offset);
                    case size_word:
                        return (Key)Script::Memory::ReadWord(offset);
                    case size_dword:
                        return (Key)Script::Memory::ReadDword(offset);
                }
                return (Key)Script::Memory::ReadQword(offset);
            }
            Value value() const
            {
                auto offset = value_ptr();
                // same as key()
                switch (parent_map->valuetype_size)
                {
                    case size_byte:
                        return (Value)Script::Memory::ReadByte(offset);
                    case size_word:
                        return (Value)Script::Memory::ReadWord(offset);
                    case size_dword:
                        return (Value)Script::Memory::ReadDword(offset);
                }
                return (Value)Script::Memory::ReadQword(offset);
            }
            size_t key_ptr() const
            {
                return node_ptr + parent_map->key_offset;
            }
            size_t value_ptr() const
            {
                return node_ptr + parent_map->value_offset;
            }
            Node left() const
            {
                auto left_addr = Script::Memory::ReadQword(node_ptr);
                return Node{left_addr, parent_map};
            }
            Node parent() const
            {
                auto parent_addr = Script::Memory::ReadQword(node_ptr + 0x8);
                return Node{parent_addr, parent_map};
            }
            Node right() const
            {
                auto right_addr = Script::Memory::ReadQword(node_ptr + 0x10);
                return Node{right_addr, parent_map};
            }
            bool color() const
            {
                return (bool)Script::Memory::ReadByte(node_ptr + 0x18);
            }
            bool is_nil() const
            {
                return (bool)Script::Memory::ReadByte(node_ptr + 0x19);
            }
            Node operator++()
            {
                if (is_nil())
                {
                    // should probably throw
                    return *this;
                }
                // get right node and check if it's valid
                Node r = right();
                if (!r.is_nil())
                {
                    // get the most left node from here
                    Node l = r.left();
                    while (!l.is_nil())
                    {
                        r = l;
                        l = r.left();
                    }

                    *this = r;
                    return *this;
                }
                // save the node for faster comparison
                // this only makes sense here as we can just compare the address
                // instead of using Read function for the nil flag
                // normally you would just check the nil flag
                Node _end = r;
                // if the right node is nil, we go up
                // if the parrent node it's nil, then that's the end
                Node p = parent();
                Node c = *this;
                while (p != _end && p.right() == c)
                {
                    // if we came from the right side, we need to go up again
                    c = p;
                    p = p.parent();
                }
                *this = p;
                return *this;
            }
            Node operator++(int)
            {
                Node old = *this;
                operator++();
                return old;
            }
            // not doing the -- operator for now as it's not really needed

            bool operator==(Node other) const
            {
                return other.node_ptr == node_ptr;
            }
            bool operator!=(Node other) const
            {
                return other.node_ptr != node_ptr;
            }
            size_t node_ptr;

          private:
            const StdMap<Key, Value>* parent_map;
        };

        size_t size() const
        {
            return Script::Memory::ReadQword(address + 0x8);
        }
        size_t at(Key v) const
        {
            Node f = find(v);
            if (f == end())
            {
                // throw std::out_of_range
            }
            return f.value();
        }
        Node begin() const
        {
            return end().left();
        }
        Node end() const
        {
            Node a{Script::Memory::ReadQword(address), this};
            return a;
        }
        Node find(Key k) const
        {
            Node _end = end();
            Node cur = root();
            while (true)
            {
                if (cur == _end)
                    return _end;

                Key node_key = cur.key();

                if (node_key == k)
                    return cur;
                else if (node_key > k)
                    cur = cur.left();
                else
                    cur = cur.right();
            }
            return _end;
        }
        bool contains(Key k) const
        {
            Node f = find(v);
            if (f == end())
                return false;
            else
                return true;
        }

      private:
        Node root() const
        {
            return end().parent();
        }
        Node last() const
        {
            return end().right();
        }
        void set_offsets(uint8_t key_alignment = alignof(Key), uint8_t value_alignment = alignof(Value))
        {
            // key and value in map are treated as std::pair
            // we need to figure out if it's placed right after the bucket flags
            // or if there is a padding added for aliment
            // the issue is, if key or value are a structs, we need to know their alignments, not just their size

            uint8_t alignment = std::max(key_alignment, value_alignment);

            switch (alignment)
            {
                case 0:
                case 1:
                case 2:
                    key_offset = 0x1A;
                    break;
                case 3:
                case 4:
                    key_offset = 0x1C;
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                    key_offset = 0x20;
                    break;
            }
            size_t offset = key_offset + keytype_size;
            // dealing with the padding between key and value
            switch (value_alignment)
            {
                case 0:
                case 1:
                    value_offset = offset;
                    break;
                case 2:
                    value_offset = (offset + 1) & ~1;
                    break;
                case 3:
                case 4:
                    value_offset = (offset + 3) & ~3;
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                    value_offset = (offset + 7) & ~7;
                    break;
            }
        }

        size_t address;
        size_t keytype_size;
        size_t valuetype_size;
        uint8_t key_offset;
        size_t value_offset;
    };
} // namespace S2Plugin
