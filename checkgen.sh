#!/bin/bash

find tpie test apps -iname '*.cpp' -or -iname '*.h' -or -iname '*.inl' | grep -v -F -f convert_log | xargs grep "new\|delete\|vector\|malloc\|free\|map\|priority_queue\|list\|unordered_map|auto_ptr" -n | grep -v "TPIE is free software" | grep -v "#include"
 
