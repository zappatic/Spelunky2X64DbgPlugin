#pragma once

#include "QtHelpers/TreeViewMemoryFields.h"
#include "ViewToolbar.h"
#include "pluginmain.h"
#include <QCheckBox>
#include <QPushButton>

namespace S2Plugin
{
    // For the proper use you need to use template to define the types
    // otherwise the functions ::find ::at ::contains ::node::key ::node::value, won't work properly
    // when not using template, you need to define the size of key and value with the correct constructor
    // this can also be used as std::set, just set the value size to 0
    // it's not the safes implementation but the one that was needed

    template <class Key = uint32_t, class Value = uint32_t> struct StdMap
    {
        StdMap(size_t addr) : address(addr){};
        // StdMap(StdMap<Key, Value>&&) = default;
        StdMap(size_t addr, size_t keySize, size_t valueSize) : address(addr), keytype_size(keySize), valuetype_size(valueSize){};

        size_t size() const
        {
            return Script::Memory::ReadQword(address + 0x8);
        }

      private:
        struct Node
        {
            Node(size_t addr, const StdMap<Key, Value>* _map) : node_ptr(addr), root(_map){};
            Node(Node& t) : node_ptr(t.node_ptr), root(t.root){};
            Key key() const
            {
                auto offset = key_ptr();
                // don't know how to use the plain Read function, so i did this
                // probably doesn't matter as for large structures you probably will just grab the address
                switch (root->keytype_size)
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
                switch (root->valuetype_size)
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
                switch (root->keytype_size + root->valuetype_size)
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
                size_t offset = root->keytype_size;
                // dealing with the padding between key and value
                switch (root->valuetype_size)
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
                return Node{left_addr, root};
            }
            Node parent() const
            {
                auto parent_addr = Script::Memory::ReadQword(node_ptr + 0x8);
                return Node{parent_addr, root};
            }
            Node right() const
            {
                auto right_addr = Script::Memory::ReadQword(node_ptr + 0x10);
                return Node{right_addr, root};
            }

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
            const StdMap<Key, Value>* root;
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

        size_t address;
        size_t keytype_size{sizeof(Key)};
        size_t valuetype_size{sizeof(Value)};
    };

    class ViewStdMap : public QWidget
    {
        Q_OBJECT
      public:
        ViewStdMap(ViewToolbar* toolbar, const std::string& keytypeName, const std::string& valuetypeName, size_t vectorOffset, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshMapContents();
        void refreshData();
        void toggleAutoRefresh(int newState);
        void autoRefreshTimerTrigger();
        void autoRefreshIntervalChanged(const QString& text);

      private:
        std::string mMapKeyType;
        std::string mMapValueType;
        size_t mmapOffset;
        size_t mMapKeyTypeSize;
        size_t mMapValueTypeSize;
        // MemoryField, offset, parrent
        std::vector<std::tuple<MemoryField, size_t, QStandardItem*>> mMemoryFields;

        QVBoxLayout* mMainLayout;
        TreeViewMemoryFields* mMainTreeView;
        ViewToolbar* mToolbar;
        QPushButton* mRefreshDataButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;

        void initializeTreeView();
        void initializeRefreshLayout();
    };
} // namespace S2Plugin
