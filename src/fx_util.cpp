#include "fx_util.h"

FXSettings* CopyFXSettings(FXSettings* aSettings)
{
    // Temporary variables
    FXint index, pos;
    FXDict* dict;
    
    // New FXSettings object
    FXSettings* newSettings = new FXSettings();
    
    // Iterate over all sections of aSettings
    for (index = aSettings->first(); index < aSettings->size(); index =
        aSettings->next(index)) 
    {   
        // Fetch current section dictionary
        dict = aSettings->data(index);
        
        // Iterate over all entries in this section
        for (pos = dict->first(); pos < dict->size(); pos = dict->next(pos)) {  
            // Copy entry from aSettings to newSettings
            newSettings->writeStringEntry(aSettings->key(index),
            dict->key(pos), (FXchar*)dict->data(pos));
        }
    }
    
    // Return copied settings
    return newSettings;
}
