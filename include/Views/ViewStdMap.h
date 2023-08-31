#pragma once

#include "pluginmain.h"

template <class Key, class Value> struct StdMap
{
    StdMap(size_t addr) : address(addr){};

    size_t size() const
    {
        return Script::Memory::ReadQword(addr + 0x8);
    }

  private:
    class Node
    {
      public:
        Key key() const
        {
            auto offset = key_ptr();
            // don't know how to use the plain Read function, so i did this
            // probably doesn't matter as for large structures you probably will just grab the address
            switch (sizeof(Key))
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
            switch (sizeof(Value))
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
            // key and value in map are treated as one struct
            // depending on the size, we need to figure out if it's placed right after the bucket flags
            // or if there is a padding added for aliment
            switch (sizeof(Key) + sizeof(Value))
            {
                case 0:
                case 1:
                case 2:
                    return node_ptr + 0x1A;
                case 3:
                case 4:
                    return node_ptr + 0x1C;
                default:
                    return node_ptr + 0x20;
            }
        }
        size_t value_ptr() const
        {
            size_t offset = sizeof(Key);
            // dealing with the padding between key and value
            switch (sizeof(Value))
            {
                case 0:
                case 1:
                    break;
                case 2:
                    offset = (offset + 1) & ~1;
                    break;
                case 3:
                case 4:
                    offset = (offset + 3) & ~3;
                    break;
                default:
                    offset = (offset + 7) & ~7;
            }
            return key_ptr() + offset;
        }

      private:
        bool is_nil() const
        {
            return (bool)Script::Memory::ReadByte(node_ptr + 0x19);
        }
        bool color() const
        {
            return (bool)Script::Memory::ReadByte(node_ptr + 0x18);
        }
        Node left() const
        {
            auto left_addr = Script::Memory::ReadQword(node_ptr);
            return Node{left_addr};
        }
        Node parent() const
        {
            auto parent_addr = Script::Memory::ReadQword(node_ptr + 0x8);
            return Node{parent_addr};
        }
        Node right() const
        {
            auto right_addr = Script::Memory::ReadQword(node_ptr + 0x10);
            return Node{right_addr};
        }

      public:
        Node operator++()
        {
            if (is_nil())
            {
                // should throw
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
    };

  public:
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
        return Node{Script::Memory::ReadQword(end().node_ptr)};
    }
    Node end() const
    {
        return Node{Script::Memory::ReadQword(address)};
    }
    Node find(Key k) const
    {
        // there is better and faster way to do this
        // but it's more code and requires the -- operator
        Node _end = end();
        for (Node cur = begin(); cur != _end; ++cur)
        {
            if (cur.key() == k)
                return cur;
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

    size_t address;
};
