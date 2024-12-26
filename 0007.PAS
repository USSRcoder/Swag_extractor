program swag7;
uses sysutils, Classes;


TYPE
  SwagHeader = packed RECORD
      HeadSize : BYTE;                  {size of header}
      HeadChk  : BYTE;                  {checksum for header}
      HeadID   : ARRAY [1..5] OF CHAR;  {compression type tag}
      NewSize  : integer;               {compressed size}
      OrigSize : integer;               {original size}
      Time     : WORD;                  {packed time}
      Date     : WORD;                  {packed date}
      Attr     : WORD;                  {file attributes and flags}
      BufCRC   : LONGINT;               {32-CRC of the Buffer }
//	  swaglen  : byte;
      Swag     : STRING[12];            {stored SWAG filename}
      Subject  : STRING[40];            {snipet subject}
      Contrib  : STRING[35];            {contributor}
      Keys     : STRING[70];            {search keys, comma deliminated}
      //FName    : PathStr;               {filename (variable length)}
      //CRC      : WORD;                  {16-bit CRC (immediately follows FName)}
    END;

var x:string;
pathlen:byte;
l:byte;
CRC      : WORD;
ss: LONGINT;

function ReadScore(var Buf:SwagHeader; Index: Integer):integer;
var
  Stream: TStream;
  i: byte;
  buffer: pchar;
  fs: TFileStream;
begin
  result := 0;
  Stream:= TFileStream.Create('ABOUT.SWG', fmOpenRead or fmShareDenyWrite);
  try
    ss := Stream.Size;

    Stream.Position:= Index;
	Stream.ReadBuffer(i, 1);
	if (i = 0) then begin
		writeln('Done!');
		result := 0;
		Exit;
	end;

    Stream.Position:= Index;
	Stream.ReadBuffer(Buf, SizeOf(SwagHeader));

	writeln('Pathlen begin at: ',Stream.Position);
	//Stream.Position := Index + 186;
	Stream.ReadBuffer(pathlen,2);
	SetLength(x,pathlen);
    l := 1;
	While l < pathlen do begin
		Stream.ReadBuffer(x[l],1);
		l:=l+1;
	end;

	Stream.ReadBuffer(CRC,2);

	GetMem ( buffer, Buf.NewSize );
	Stream.read ( buffer^, Buf.NewSize );

	fs := TFileStream.Create('C:\Old\distr\SWAG\ffe\'+inttostr(Index)+'.my', fmCreate);
	try
		fs.WriteBuffer(buffer^, Buf.NewSize);
		writeln('write to: ','C:\Old\distr\SWAG\ffe\'+inttostr(Index)+'.my ' , Buf.NewSize);
    finally
        fs.Free;
    end;

	FreeMem ( buffer );
	
  finally
    Stream.Free;
  end;
  result := 1;
end;

var rank:  SwagHeader;

ofs:LONGINT;
begin
	rank.Swag:= '';
	ofs:=0;
	ss := 1;
	while (ReadScore(rank, ofs) = 1) do begin
		WriteLn('HeadSize :',rank.HeadSize );
		WriteLn('HeadCRC  :',rank.HeadChk  );
		WriteLn('HeadID   :',rank.HeadID   );
		WriteLn('Comp.size:', rank.NewSize );
		WriteLn('OrigSize :',rank.OrigSize );
		WriteLn('Time     :',rank.Time     );
		WriteLn('Date     :',rank.Date     );
		WriteLn('Attr     :',rank.Attr     );
		WriteLn('BufCRC   :',rank.BufCRC   );
		WriteLn('Swag     :',rank.Swag     );
		WriteLn('Subject  :',rank.Subject  );
		WriteLn('Contrib  :',rank.Contrib  );
		WriteLn('Keys     :',rank.Keys     );
		WriteLn('pathlen  :',pathlen  );
		WriteLn('x        :',x  );
		WriteLn('CRC      :',CRC);
		ofs := ofs + rank.HeadSize + rank.NewSize + 2;
		WriteLn('#        :',ofs);
		WriteLn('----------------');
	end;
end.