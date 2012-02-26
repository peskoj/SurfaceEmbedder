{$B-}{$I+}{$R-}
program kubicni;
{uses sysutils;}

{ Iskanje kubicnih prepovedanih podgrafov za torus. }
{ Izboljsani algoritem za rod. Spremembe od splosnega programa so le v
  proceduri RodGrafa }

  const
    potenca: array [0..5] of word = (1,2,4,8,16,32);
  const
    maxTc = 30;      { maksimalno "stevilo to"ck }
    maxPvz = 45;     { maksimalno "stevilo povezav }
    maxLic = 30;     { maksimalno "stevilo lic pri vlo"zitvi v torus }
    datoteka = 'GRAFI.TXT'; { privzeta vhodna datoteka }
    stGrafov: longint = 0;  { stevilo testiranih grafov }
    stTrik: longint = 0;    { stevilo grafov s trikotnikom }
    stVloz: longint = 0;    { stevilo grafov z vlozitvijo v torus }
    stOvir: longint = 0;    { stevilo grafov brez vlozitve v torus }
    stMinOvir: longint = 0; { stevilo minimalnih ovir }
    maxSt = 3;       { maksimalna stopnja to"cke }

  type
    oznaka = byte;
    povezava = array [boolean] of oznaka;
    mnozica = set of 1..maxTc;
    graf = array [1..maxTc] of { tabela to"ck s seznami sosedov }
             array [1..maxSt] of oznaka;
  var
    { podatki o grafu }
    opis: string[255];
    m,n: oznaka; { "stevilo povezav in "stevilo to"ck }
    pvz: array [1..maxPvz] of povezava; { tabela povezav }
    tocke: graf;
    stop: array [1..maxTc] of oznaka; { tabela stopenj to"ck }
    sosedi: array [1..maxTc] of mnozica; { mno"zice sosedov }
    { podatki o vlo"zitvi }
    vlozG: array [1..maxTc] of { vlo"zitev celotnega grafa }
             array [1..maxSt+1] of oznaka; { "se prostor za stra"zarja }
    { analiza vlo"zitve }
    polpvz: array [1..maxPvz] of povezava; { mesto povezave v lok. rotacijah }
    pregled: array [1..maxPvz] of array [boolean] of boolean;
    { odkrivanje napak }
    ovira: boolean;
    rod: integer;
    v1,v2,u1,u2: oznaka;
    i,j1,j2,j,k: oznaka;
    e: povezava;
    vhod_ime, izhod_ime: string[255];
    vhod, izhod: text;  { podatki z grafi in izpis ovir }
    napake: text; { datoteka ERROR.TXT }
    cas_ura,cas_min,cas_sek,cas_sto: word;
    cas: real;
    zacetni, koncni: longint;


procedure Napaka(koda: integer; b: boolean);
{ Izpi"se obvestilo o napaki, ki ustreza kodi. }
{ Logi"cna spremenljivka preklaplja med napakami in opozorili. }
  var niz: string[255];
begin
  if b then koda := koda+100;
  { Izpis na ERROR.TXT }
  append(napake);
  writeln(napake,'Napaka stevilka ',koda:0,'.   Stevilka grafa = ',stGrafov);
  flush(napake);
  close(napake);
  if koda=0 then
    writeln('Testni izpis -- ni napake.')
  else begin
    case koda of
       1: niz := 'Napacno stevilo vrst grafov';
       2: niz := 'Premalo ali prevec vlozitev grafa';
       3: niz := 'Premalo podatkov v opisu grafa';
       5: niz := 'Preimenuj - tocka ni krajisce dane povezave';
       6,7: niz := 'Stevilo pojavitev ni med 1 in 3';
       104: niz := 'Graf ni kubicen';
      else
        niz := 'neznana napaka';
    end; {case}
    if not b then
      writeln(chr(7),'   NAPAKA ',koda,' -- ',niz,'.')
    else
      writeln('   OPOZORILO ',koda,' -- ',niz,'.');
  end; {else}
end; {Napaka}

procedure Pretvori(niz: string);
{ Iz "tekstovnega" opisa grafa doloci vrednosti globalnih }
{ spremenljivk n, m, tocke, pvz, stop, sosedi.            }
  var
    i,j: integer;
    iznak,ipot: integer;
    zlog: byte;
begin
  n := ord(niz[1])-63;
  for i:=1 to maxTc do begin
    sosedi[i] := [];
    for j:=1 to maxSt do tocke[i][j] := 0;
    stop[i] := 0;
  end;
  m := 0;
  iznak := 1; ipot := 0;
  for j:=2 to n do { po vseh parih tock }
    for i:=1 to j-1 do begin
      ipot := ipot-1;
      if ipot=-1 then begin
        ipot := 5;
        iznak := iznak+1;
        if iznak>length(niz) then Napaka(3,false);
        zlog := ord(niz[iznak])-63;
      end;
      if (zlog and potenca[ipot]) <> 0 then begin
        { povezava med i in j, i<j }
        sosedi[i] := sosedi[i]+[j];  sosedi[j] := sosedi[j]+[i];
        inc(stop[i]);  inc(stop[j]);
        m := m+1;
        pvz[m][true] := i;  pvz[m][false] := j;
        tocke[i][stop[i]] := m;
        tocke[j][stop[j]] := m;
      end;
    end; {for i,j}

    for i:=m+1 to maxPvz do
      begin pvz[i][true] := 0;  pvz[i][false] := 0; end;

    if 3*n<>2*m then Napaka(4,true);
    for i:=1 to n do if stop[i]<>3 then Napaka(4,true);
end; {Pretvori}


procedure IzpisOvire;
{ Izpise trenutni graf (min oviro) na izhod }
  var i,j,k: oznaka;
begin
  { Opis grafa: % stOvire/stGrafov pri delu in originalni opis }
  writeln(izhod,'% ',stMinOvir:0,' / ',stGrafov:0,'   ',opis);
  writeln(izhod,n:3,m:4);
  for i:=1 to n do begin
    write(izhod,i:3,' ');
    for j:=1 to stop[i] do begin
      k:=tocke[i][j];
      write(izhod,pvz[k][pvz[k][false]=i]:3);
    end;
    writeln(izhod,'  0');
  end;
  writeln(izhod);
end; { IzpisOvire }


function Trikotnik: boolean;
{ Ugotovi, ali graf vsebuje trikotnik.                     }
{ Uporablja globalne spremenljivke n, tocke, stop, sosedi. }
  label
    konecTrikotnik;
  var
    i,j,k: oznaka;
    f,v1,v2: oznaka;
begin
  Trikotnik := false;
  for i:=1 to n-1 do
    for j:=1 to stop[i]-1 do
      for k:=j+1 to stop[i] do begin
        f := tocke[i][j];  v1 := pvz[f][true]+pvz[f][false]-i;
        f := tocke[i][k];  v2 := pvz[f][true]+pvz[f][false]-i;
        if v1 in sosedi[v2] then
          begin Trikotnik := True; goto konecTrikotnik; end;
      end; {for i,j,k}
  konecTrikotnik:
end; {Trikotnik}


function RodGrafa: integer;
{ Ugotovi (orientabilni) rod grafa. Za ravninski graf vrne 1.      }
{ Uporablja globalne spremenljivke n, m, tocke, pvz, stop, sosedi. }
{ IZBOLJSANI ALGORITEM }
  var
    i,j,k,l,l1,t: oznaka;
    rod, minrod: integer;
    stLic, jlic: integer;  { jlic = "stevilo" lic pri j-ju }
    stevilopojavitev: integer;
    Euler: integer;
    smer,smer1: boolean;
    konec: Boolean;
    MoznaSpr: integer;  { Maksimalna mozna sprememba roda }
begin
  { generiraj vlo"zitve celotnega grafa }
  { kje nastopajo v rotacijah posamezne povezave }
  for i:=1 to n do begin
    for j:=1 to stop[i] do
      vlozG[i][j] := tocke[i][j];
    vlozG[i][stop[i]+1] := vlozG[i][1];
  end;
  for j:=1 to n do
    for k:=1 to stop[j] do
      if pvz[vlozG[j][k]][true]=j then
        polpvz[vlozG[j][k]][true] := k
      else
        polpvz[vlozG[j][k]][false] := k;

  { Izracunaj zacetno Eulerjevo karakteristiko }
  { poisci stevilo lic }
  for l:=1 to m do { vse pvz nepregledane }
    begin pregled[l][true] := false; pregled[l][false] := false; end;
  stLic := 0;
  l1 := 1; { "stevec po tabeli povezav }
  smer := true;
  repeat { Dolocamo stevilo lic. }
    { pregledamo teko"ce lice }
    l := l1;
    repeat { sprehod po robu lica } { upo"stevamo, da ni zank }
      { teko"ca povezava je l } { teko"ca smer je smer }
      pregled[l][smer] := true; { ozna"cimo teko"co povezavo }
      t := pvz[l][not smer]; { drugo kraji"s"ce trenutne povezave }
      l := vlozG[t][polpvz[l][not smer]+1]; { naslednja povezava }
      smer := pvz[l][true]=t; { dolo"cimo novo smer }
    until pregled[l][smer];
    inc(stLic);
    { poi"s"cemo naslednjo nepregledano povezavo }
    while (l1<=m) and pregled[l1][true] and pregled[l1][false] do
      inc(l1);
    if l1<=m then
      smer := not pregled[l1][true];
  until l1>m; { Dolocili smo stevilo lic. }
  Euler := n-m+stLic;

  minrod := 2*n;

  repeat { po vseh vlozitvah, dokler ne najdes rod <=1 }
    rod := 1-(Euler div 2);
    if rod<minrod then minrod:=rod;
    if rod<0 then begin
      writeln(chr(7),'NAPAKA -- premajhen rod.');
    end;

    if minrod<=1 then break;  { graf je roda <=1 in izhod }

    { poi"s"ci naslednjo vlo"zitev }
    konec := false;
    j :=n;
    while (not konec) and (j>1) do begin
      { Zadnji pogoj: za kubicni graf le ena permutacija v j=1 }
      { Posku"samo pove"cati permutacijo pri j-ti to"cki }
      { Najprej ugotovimo, v koliko licih je tocka j }
      { Prehodi prvo njeno lice in doloci stevilo pojavitev j-ja }
      { ce stevilo pojavitev=3, jlic:=1 sicer pa 3 (bo OK tudi za 2!) }
      l1 := vlozG[j][1];
      smer1 := pvz[l1][true]=j;
      smer:=smer1;
      l:=l1; { tekoca povezava ob prehodu lica }
      stevilopojavitev := 0;
      repeat { sprehod po robu lica } { upo"stevamo, da ni zank }
        { teko"ca povezava je l } { teko"ca smer je smer }
        t := pvz[l][not smer]; { drugo kraji"s"ce trenutne povezave }
        if t=j then inc(stevilopojavitev);
        l := vlozG[t][polpvz[l][not smer] + 1]; { naslednja povezava }
        smer := pvz[l][true]=t; { dolo"cimo novo smer }
      until (l=l1) and (smer=smer1);
      if stevilopojavitev=3 then jlic:=1 else jlic:=3;
      if not(stevilopojavitev in [1..3]) then
         Napaka(6,false);

      { Sestopamo z j, dokler originalne rotacije ne obrnemo v drugo
        in da je trenutni rod se vedno mozno spraviti pod 2 }
      if jlic = 1 then MoznaSpr:=n-j+1 else MoznaSpr:=n-j;
      if (vlozG[j][3] > vlozG[j][2]) and (rod-MoznaSpr<2) then konec := true;
      if konec then begin
        { zamenjam povezavi 2 in 3 v rotaciji tocke j }
        t := vlozG[j][2];
        vlozG[j][2] := vlozG[j][3];
        vlozG[j][3] := t;
      end
      else if stop[j]=3 then begin { rotacijo v j postavimo na originalno }
        vlozG[j][2] := tocke[j][2];
        vlozG[j][3] := tocke[j][3];
      end;

      for k:=2 to stop[j] do
        if pvz[vlozG[j][k]][true]=j then
          polpvz[vlozG[j][k]][true] := k
        else
          polpvz[vlozG[j][k]][false] := k;
      { doloci novo Eulerjevo karakteristiko - le spremembo celic pri j }
      { stevilo lic, v katerih je j }
      Euler := Euler - jlic;

      { V koliko licih nastopa tocka j po spremembi }
      { jlic := 1, ce j v 1 licu, jlic := 3, ce j v 2 ali 3 licih }
      { pregledamo prvo lice, ki vsebuje j }
      if stevilopojavitev = 2 then jlic:=3
      else begin
        l1 := vlozG[j][1];
        smer1 := pvz[l1][true]=j;
        smer:=smer1;
        l:=l1; { tekoca povezava ob prehodu lica }
        stevilopojavitev := 0;
        repeat { sprehod po robu lica } { upo"stevamo, da ni zank }
          { teko"ca povezava je l } { teko"ca smer je smer }
          t := pvz[l][not smer]; { drugo kraji"s"ce trenutne povezave }
          if t=j then inc(stevilopojavitev);
          l := vlozG[t][polpvz[l][not smer] + 1]; { naslednja povezava }
          smer := pvz[l][true]=t; { dolo"cimo novo smer }
        until (l=l1) and (smer=smer1);
        if stevilopojavitev=3 then jlic:=1 else jlic:=3;
        if not(stevilopojavitev in [1..3]) then
           Napaka(7,false);
      end;
      Euler := Euler + jlic;
      j:=j-1;
    end; {while} { konec=true, uspeli pove"cati }
  until not konec; { zadnja vlo"zitev }

  RodGrafa := minrod;
end; {RodGrafa}

procedure Preimenuj(u: oznaka; staro, novo: oznaka);
{ V seznamu povezav pri tocki u preimenuje povezavo }
{ staro v novo. Nato uredi tabelo povezav.          }
  var
    i,j: oznaka;
    x: oznaka;
begin
  i := 1;
  while (i<=stop[u]) and (tocke[u][i]<>staro) do i := i+1;
  if tocke[u][i]<>staro then Napaka(5,false);
  tocke[u][i] := novo;
  { Urejanje z vstavljanjem brez strazarja. }
  for i:=2 to stop[u] do begin
    x := tocke[u][i];
    j := i-1;
    while (j>0) and (tocke[u][j]>x) do
      begin tocke[u][j+1] := tocke[u][j];  j := j-1; end;
    tocke[u][j+1] := x;
  end;
end; {Preimenuj}


begin { glavni prg }
  assign(napake,'ERROR.TXT');
  rewrite(napake);
  close(napake);
  {***}
  write('Datoteka z opisi grafov: '); readln(vhod_ime);
  if vhod_ime='' then vhod_ime := datoteka;
  if Pos('.',vhod_ime)=0 then
    vhod_ime := 'podatki\kub'+vhod_ime+'.txt';
  assign(vhod,vhod_ime);
  {$I-} reset(vhod); {$I+}
  if IOResult<>0 then begin
    writeln(chr(7),'Ne najdem datoteke ',vhod_ime,'.');
  end;
  {***}
  write('Izhodna datoteka: '); readln(izhod_ime);
  assign(izhod,izhod_ime);
  rewrite(izhod);
  {***}
  writeln('Zacetni graf, ki ga naj pregledam: '); readln(zacetni);
  writeln('Konc"ni graf, ki ga naj pregledam: '); readln(koncni);

{  gettime(cas_ura,cas_min,cas_sek,cas_sto);
  cas := 3600.0*cas_ura+60*cas_min+cas_sek+cas_sto/100;}

  while not eof(vhod) do begin
    readln(vhod,opis); { Preberemo tekstovni opis grafa. }
    stGrafov := stGrafov+1;
    Pretvori(opis); { Pripravi podatke o grafu. }
    if Trikotnik then
      stTrik := stTrik+1
    else
    if (stVloz+1<zacetni) or (stVloz+stOvir+1>koncni) then
      stVloz:=stVloz+1
    else begin
      writeln('Obdelujem graf ',stGrafov);
      rod := RodGrafa; { Izracunamo rod grafa. }
      if rod<2 then
        stVloz := stVloz+1 { Ima vlozitev v torus. }
      else begin
        stOvir := stOvir+1; { Graf je ovira. }
        if rod=2 then begin { Minimalna ovira ima rod 2. }
          { Za vsako povezavo e preverimo, ali ima G-e vlozitev v torus. }
          ovira := true;
          for i:=1 to m do begin
            { Odstranimo povezavo i: }
            v1 := pvz[i][true];  v2 := pvz[i][false];
            e := pvz[i];  pvz[i] := pvz[m];
            m := m-1;
            stop[v1] := stop[v1]-1;  stop[v2] := stop[v2]-1;
            sosedi[v1] := sosedi[v1]-[v2];
            sosedi[v2] := sosedi[v2]-[v1];
            { Popravimo oba seznama sosedov. }
            j1 := 1;  while tocke[v1][j1]<>i do j1 := j1+1;
            for k:=j1 to stop[v1] do tocke[v1][k] := tocke[v1][k+1];
            j2 := 1;  while tocke[v2][j2]<>i do j2 := j2+1;
            for k:=j2 to stop[v2] do tocke[v2][k] := tocke[v2][k+1];
            { Zadnjo povezavo preimenujemo v i. }
            Preimenuj(pvz[m+1][true],m+1,i);
            Preimenuj(pvz[m+1][false],m+1,i);

            if RodGrafa>1 then ovira := false;

            { Vrnemo povezavo i: }
            Preimenuj(pvz[m+1][true],i,m+1);
            Preimenuj(pvz[m+1][false],i,m+1);
            m := m+1;
            pvz[i] := e;
            stop[v1] := stop[v1]+1;  stop[v2] := stop[v2]+1;
            sosedi[v1] := sosedi[v1]+[v2];
            sosedi[v2] := sosedi[v2]+[v1];
            { Vstavimo v1 in v2 nazaj v seznama sosedov. }
            for k:=stop[v1]-1 downto j1 do tocke[v1][k+1] := tocke[v1][k];
            tocke[v1][j1] := i;
            for k:=stop[v2]-1 downto j2 do tocke[v2][k+1] := tocke[v2][k];
            tocke[v2][j2] := i;

            if not ovira then break;
          end; {for}
          if ovira then begin
            stMinOvir := stMinOvir+1;
            writeln('Graf stevilka ',StGrafov:6,' je minimalna ovira');
            IzpisOvire;
          end;
        end; {rod=2}
      end; {else rod<2}
    end; {else Trikotnik}
  end; {while}

{  gettime(cas_ura,cas_min,cas_sek,cas_sto);
  cas := 3600.0*cas_ura+60*cas_min+cas_sek+cas_sto/100-cas;}

  writeln;
  writeln(' n = ',n:0); 
  writeln('   Stevilo grafov: ',StGrafov:20,'.');
  writeln('   Stevilo grafov s trikotnikom: ',StTrik:6,'.');
  writeln('   Stevilo grafov roda <= 1: ',StVloz:10,'.');
  writeln('   Stevilo grafov roda >= 2: ',StOvir:10,'.');
  writeln('   Stevilo minimalnih ovir: ',StMinOvir:11,'.');
  If StGrafov <> StTrik+StOvir+StVloz then Napaka(1,false);
{  writeln('   Porabljeni cas: ',cas:0:2,' sekund.');
  writeln('   Povprecje: ',StGrafov/cas:0:2,' grafov na sekundo.');}

  writeln(izhod);
  writeln(izhod,'   Stevilo grafov roda <= 1: ',StVloz:10,'.');
  writeln(izhod,'   Stevilo grafov roda >= 2: ',StOvir:10,'.');
  writeln(izhod,'   Stevilo minimalnih ovir: ',StMinOvir:11,'.');
  If StGrafov <> StTrik+StOvir+StVloz then Napaka(1,false);
{  writeln(izhod,'   Porabljeni cas: ',cas:0:2,' sekund.');}

  close(vhod);
  close(izhod);
end.
