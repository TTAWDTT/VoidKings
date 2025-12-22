# Train Panel Layout Adjustment

Date: 2025-12-23  
Author: Codex

## Goal
- Fix slight offset in the training panel layout

## Summary
- Center the panel using anchor-based positioning
- Add a panel-level offset to move the full window to the upper-right
- Add inner padding so title bar, resource text, and card grid align
- Center the card grid within the padded area
- Shift the whole layout using a dedicated content root node
- Ensure the card list starts at the top of the scroll area

## Files
- Classes/UI/TrainPanel.cpp
- Classes/UI/TrainPanel.h
