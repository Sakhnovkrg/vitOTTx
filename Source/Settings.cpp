/* Copyright 2026 Alexandr Sakhnov
 *
 * This file is part of vitOTTx.
 *
 * vitOTTx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vitOTTx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with vitOTTx. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Settings.h"

#include <memory>

namespace vitottx::Settings
{
namespace
{
juce::PropertiesFile& globals()
{
    static auto props = []
    {
        juce::PropertiesFile::Options o;
        o.applicationName     = "vitOTTx";
        o.filenameSuffix      = "settings";
        o.folderName          = "vitOTTx";
        o.osxLibrarySubFolder = "Application Support";
        o.storageFormat       = juce::PropertiesFile::storeAsXML;
        return std::make_unique<juce::PropertiesFile>(o);
    }();
    return *props;
}
} // anon

int getEditorWidth(int fallback)
{
    return globals().getIntValue("editorWidth", fallback);
}

void setEditorWidth(int width)
{
    globals().setValue("editorWidth", width);
    globals().save();
}

} // namespace vitottx::Settings
