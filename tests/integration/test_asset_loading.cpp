// ======================================================================
// Integration Test: Asset Loading Pipeline
// ======================================================================
// Tests the complete asset loading system including:
//   - File discovery and parsing
//   - Texture loading (DDS files)
//   - Font loading
//   - JSON configuration files
//   - Sound file enumeration
//
// REQUIREMENTS:
//   - gamedata/ directory available at runtime
//   - Valid asset files present
//
// This test runs with working directory set to bin/<Config>/ where
// gamedata/ is mirrored by the build system.
// ======================================================================

#include "TestHarness.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Mock asset loader for demonstration
class AssetDatabase
{
  private:
    fs::path m_gamedataPath;
    std::vector<std::string> m_loadedAssets;

  public:
    explicit AssetDatabase(const fs::path& gamedataPath)
      : m_gamedataPath(gamedataPath)
    {
    }

    bool ValidateGamedataDirectory() const
    {
      return fs::exists(m_gamedataPath) && fs::is_directory(m_gamedataPath);
    }

    bool LoadPreferences()
    {
      fs::path prefPath = m_gamedataPath / "preferences.txt";
      if (!fs::exists(prefPath))
        return false;

      std::ifstream file(prefPath);
      if (!file.is_open())
        return false;

      m_loadedAssets.push_back("preferences.txt");
      return true;
    }

    bool LoadStrings()
    {
      fs::path stringsPath = m_gamedataPath / "Strings-ENG.json";
      if (!fs::exists(stringsPath))
        return false;

      // In real implementation: parse JSON with nlohmann_json
      std::ifstream file(stringsPath);
      if (!file.is_open())
        return false;

      m_loadedAssets.push_back("Strings-ENG.json");
      return true;
    }

    int EnumerateFonts()
    {
      fs::path fontsPath = m_gamedataPath / "Fonts";
      if (!fs::exists(fontsPath))
        return 0;

      int count = 0;
      for (const auto& entry : fs::directory_iterator(fontsPath))
      {
        if (entry.path().extension() == ".dds")
        {
          m_loadedAssets.push_back(entry.path().filename().string());
          ++count;
        }
      }
      return count;
    }

    int EnumerateIcons()
    {
      fs::path iconsPath = m_gamedataPath / "Icons";
      if (!fs::exists(iconsPath))
        return 0;

      int count = 0;
      for (const auto& entry : fs::directory_iterator(iconsPath))
      {
        if (entry.path().extension() == ".dds")
        {
          ++count;
        }
      }
      return count;
    }

    bool ValidateLevelDirectory()
    {
      fs::path levelsPath = m_gamedataPath / "Levels";
      return fs::exists(levelsPath) && fs::is_directory(levelsPath);
    }

    size_t GetLoadedAssetCount() const
    {
      return m_loadedAssets.size();
    }
};

int main()
{
  io::tests::TestContext ctx("Integration::AssetLoading");

  // Working directory should be bin/<Config>/ where gamedata/ is mirrored
  fs::path gamedataPath = fs::current_path() / "gamedata";

  // Test 1: Gamedata directory exists
  {
    bool exists = fs::exists(gamedataPath);
    IO_EXPECT_TRUE(ctx, exists);

    if (!exists)
    {
      // Provide helpful error message for debugging
      std::cerr << "ERROR: gamedata/ not found at: " << gamedataPath << '\n';
      std::cerr << "Current directory: " << fs::current_path() << '\n';
      std::cerr << "This test must run from bin/<Config>/ where gamedata/ is mirrored.\n";
    }
  }

  // Test 2: Load preferences file
  {
    AssetDatabase db(gamedataPath);
    bool loaded = db.LoadPreferences();
    
    IO_EXPECT_TRUE(ctx, loaded);
  }

  // Test 3: Load localization strings
  {
    AssetDatabase db(gamedataPath);
    bool loaded = db.LoadStrings();
    
    IO_EXPECT_TRUE(ctx, loaded);
  }

  // Test 4: Enumerate font assets
  {
    AssetDatabase db(gamedataPath);
    int fontCount = db.EnumerateFonts();
    
    // Should find at least the English fonts
    // (EditorFont-ENG.dds, SpeccyFont-ENG.dds, and possibly others)
    IO_EXPECT_TRUE(ctx, fontCount >= 2);
  }

  // Test 5: Enumerate icon assets
  {
    AssetDatabase db(gamedataPath);
    int iconCount = db.EnumerateIcons();
    
    // Should find many icons (background, banners, unit icons, etc.)
    IO_EXPECT_TRUE(ctx, iconCount > 10);
  }

  // Test 6: Validate level directory structure
  {
    AssetDatabase db(gamedataPath);
    bool valid = db.ValidateLevelDirectory();
    
    IO_EXPECT_TRUE(ctx, valid);
  }

  // Test 7: Multiple asset loads accumulate
  {
    AssetDatabase db(gamedataPath);
    
    size_t initialCount = db.GetLoadedAssetCount();
    IO_EXPECT_TRUE(ctx, initialCount == 0);
    
    db.LoadPreferences();
    db.LoadStrings();
    db.EnumerateFonts();
    
    size_t finalCount = db.GetLoadedAssetCount();
    IO_EXPECT_TRUE(ctx, finalCount > 0);
  }

  // Test 8: Critical files exist
  {
    std::vector<fs::path> criticalFiles = {
      gamedataPath / "game.txt",
      gamedataPath / "locations.txt",
      gamedataPath / "Strings-ENG.json",
      gamedataPath / "preferences.txt",
      gamedataPath / "Fonts" / "EditorFont-ENG.dds",
      gamedataPath / "Icons" / "background.dds",
    };

    for (const auto& file : criticalFiles)
    {
      bool exists = fs::exists(file);
      IO_EXPECT_TRUE(ctx, exists);
      
      if (!exists)
      {
        std::cerr << "Missing critical file: " << file << '\n';
      }
    }
  }

  return ctx.Finalize();
}

// ======================================================================
// ASSET LOADING REFACTORING RECOMMENDATIONS
// ======================================================================
//
// Current Pain Points:
//
// 1. FILE I/O IN CONSTRUCTORS
//    Problem: Classes load files in constructors, can't test without filesystem
//    Solution: 
//      - Introduce IFileSystem interface
//      - MemoryFileSystem for testing
//      - DiskFileSystem for production
//
// 2. HARD-CODED PATHS
//    Problem: Paths like "gamedata/textures/foo.dds" scattered everywhere
//    Solution:
//      - AssetPath resolver class
//      - Configuration-driven asset paths
//      - Virtual file system with mount points
//
// 3. SYNCHRONOUS LOADING
//    Problem: All loading blocks main thread
//    Solution:
//      - Async loading with std::async or custom task system
//      - Asset streaming for large resources
//      - Priority queue for critical assets
//
// 4. NO ASSET VALIDATION
//    Problem: Crashes if assets are corrupted or missing
//    Solution:
//      - Validate assets at startup
//      - Fallback assets for missing files
//      - Detailed error reporting
//
// Example refactored interface:
//
// class IFileSystem {
//   virtual std::vector<uint8_t> ReadFile(const fs::path& path) = 0;
//   virtual bool FileExists(const fs::path& path) = 0;
// };
//
// class AssetLoader {
//   AssetLoader(IFileSystem* fs, const fs::path& basePath);
//   std::future<Texture*> LoadTextureAsync(const std::string& name);
//   void ValidateAllAssets();  // Call at startup
// };
//
// ======================================================================
