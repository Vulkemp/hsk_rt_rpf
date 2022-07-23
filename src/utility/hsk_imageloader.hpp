#pragma once
#include "../hsk_basics.hpp"
#include "hsk_imageformattraits.hpp"
#include <functional>
#include <vulkan/vulkan.h>

namespace hsk {

    enum class EImageChannel
    {
        Unknown = -1,
        R       = 0,
        G,
        B,
        A,
        Y
    };

    /// @brief General purpose image loader
    template <VkFormat FORMAT>
    class ImageLoader
    {
      public:
        using FORMAT_TRAITS = ImageFormatTraits<FORMAT>;

        class ImageInfo : public NoMoveDefaults
        {
          public:
            using FORMAT_TRAITS = ImageFormatTraits<FORMAT>;

            bool                       Valid     = false;
            std::string                Utf8Path  = "";
            std::string                Extension = "";
            std::string                Name      = "";
            std::vector<EImageChannel> Channels;
            VkExtent2D                 Extent = VkExtent2D{};
        };

        inline ImageLoader() {}

        /// @brief Inits the image loader
        /// @return True if the image exists, can be loaded and the data is suitable for the
        inline bool Init(std::string_view utf8path);

        /// @brief Checks if format the loader was initialized in supports linear tiling transfer and shader read
        inline static bool sFormatSupported(const VkContext* context);

        /// @brief Loads the file into CPU memory (Init first!)
        inline bool Load();

        /// @brief Cleans up the loader
        inline void Cleanup();

        virtual inline ~ImageLoader() { Cleanup(); }

        HSK_PROPERTY_ALLGET(Info)
        HSK_PROPERTY_ALLGET(RawData)


      protected:
        ImageInfo                  mInfo;
        std::vector<uint8_t>       mRawData;
        void*                      mCustomLoaderInfo        = nullptr;
        std::function<void(void*)> mCustomLoaderInfoDeleter = {};

        bool PopulateImageInfo_TinyExr();
        bool PopulateImageInfo_Stb();
        bool Load_TinyExr();
        bool Load_Stb();
    };

}  // namespace hsk