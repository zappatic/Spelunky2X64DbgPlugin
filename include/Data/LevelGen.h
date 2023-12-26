#pragma once

#include <QColor>
#include <unordered_map>

namespace S2Plugin
{
    struct State;

    struct RoomCode
    {
        uint16_t id;
        std::string name;
        QColor color;
    };

    class LevelGen
    {
      public:
        LevelGen(State* state);
        void setState(State* state);
        bool loadLevelGen();

        std::unordered_map<std::string, size_t>& offsets();
        void refreshOffsets();
        size_t offsetForField(const std::string& fieldName) const;
        std::string themeNameOfOffset(size_t offset) const;
        RoomCode roomCodeForID(uint16_t code) const;

        void reset();

      private:
        size_t mLevelGenPtr = 0;
        State* mState;
        std::unordered_map<std::string, size_t> mMemoryOffsets; // fieldname -> offset of field value in memory
        std::unordered_map<uint16_t, RoomCode> mRoomCodes;

        void processJSON();
    };
} // namespace S2Plugin
