::@echo off 

@for /d /r %%c in (Debug,Release,nor_zf_ram) do @if exist %%c ( rd /s /q "%%c" & echo     删除目录%%c) 

:: *.ewd 是下载配置文件
pause