# Add the BittyBuzz base BST file.
file(GLOB BBZ_BASE_BST_FILE "*.bst")
set(BBZ_BASE_BST_FILE "${BBZ_BASE_BST_FILE}" CACHE PATH
    "Path to BittyBuzz's Buzz String Table (.bst) file.")