::@echo off 

@for /d /r %%c in (Debug,Release,nor_zf_ram) do @if exist %%c ( rd /s /q "%%c" & echo     ɾ��Ŀ¼%%c) 

:: *.ewd �����������ļ�
pause