
//! (Big Thanks To Tactical Data Link For Sending Me This Information) 
//! CSI_CoalitionSquadInterface Preprocessor Directives
//! 
//! This file handles conditional compilation flags for optional mod dependencies.
//! It's named with "!" prefix to ensure it's compiled before other scripts.
//! Defines are only valid in the module they are defined in, thus this script is defined in Scripts/Game, so references in other modules will fail.
//! 
//! For mod developers:
//! - If your mod may interface with this one, but you don't have this mod as a dependency, wrap code which interfaces with this mod using:
//!
//! #ifdef CSI_CoalitionSquadInterface
//!     // Your Code Here
//! #endif
//!
//! This ensures that the code is compiled only when the flag is set. If it's not, which it won't be if this mod is not loaded, it will ignore that code as if it never existed.

#define CSI_CoalitionSquadInterface