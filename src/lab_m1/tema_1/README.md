# Joc de carti 1v1, inspirat de Hearthstone

Jucatorul de jos este controlat cu mouse-ul, iar adversarul de sus este controlat de calculator.

## Comenzi

- Muta cursorul peste o carte vizibila pentru a o mari lin la 110%; cartea din mana proprie se ridica deasupra randului.
- O carte selectata pentru drag sau atac se mareste la 118%; la deselectare revine treptat.
- Trage o carte din mana pe jumatatea de jos a tablei. Costul se scade doar la plasarea valida.
- Alege prin click o creatura proprie cu contur verde, apoi o creatura adversa sau eroul advers pentru atac.
- In timpul alegerii tintei, o sageata urmareste cursorul si devine verde peste o tinta valida. Click dreapta anuleaza alegerea atacatorului.
- Apasa **SFARSIT TURA** pentru a lasa calculatorul sa joace.
- **R** incepe un meci nou. La final exista si butonul **JOC NOU**.

Fiecare carte vizibila are **doua sabii incrucisate in stanga-jos pentru atac**,
o **inima rosie in dreapta-jos pentru viata ramasa** si un **cristal albastru fatetat in stanga-sus pentru costul de mana**.
Fundalul dreptunghiular este rosu pentru luptator, albastru pentru mag si verde pentru garda.
Un contur verde indica o carte din mana care poate fi jucata sau o creatura care poate ataca;
conturul galben marcheaza cartea trasa sau atacatorul ales.

Mana ambilor eroi foloseste aceleasi cristale fatetate: albastre pentru resursele disponibile
si gri pentru cele consumate. Zonele de selectie ale cristalelor de pe carti urmeaza
conturul poligonal, inclusiv varfurile iesite din carte si scalarea la hover/selectare.

Eroii sunt centrati sus si jos. Mana proprie este grupata pe un arc discret in dreapta-jos,
iar mana adversa este in stanga-sus. Cartile jucate sunt centrate pe doua randuri compacte,
cu pachetele si butonul de tura in dreapta tablei. Pozitiile animate si zonele de selectie
folosesc aceeasi asezare, inclusiv marginile bulinelor care ies din conturul cartii.

## Reguli si alegeri pentru aceasta versiune

- Ambii jucatori au aceeasi compozitie de 30 de carti (sase modele repetate de cinci ori), amestecate independent la fiecare meci, inainte de impartirea mainilor. Ordinea pachetului ramas este astfel aleatoare.
- Fiecare tura dureaza cel mult 30 de secunde, inclusiv animatiile. Inelul din stanga tablei se goleste in sens orar, afiseaza secundele ramase in centru si devine rosu in ultimele cinci secunde. La expirare, tura se incheie automat; o carte tinuta cu mouse-ul ramane in mana fara cost.
- Inima foloseste curba parametrica clasica `x = 16 sin^3(t)`, `y = 13 cos(t) - 5 cos(2t) - 2 cos(3t) - cos(4t)`. Sabiile sunt compuse procedural din lame cu varf ascutit, muchii argintii, garzi si manere, rotite simetric. Cercurile folosesc raza constanta.
- Initial, fiecare are cinci carti in mana, 25 in pachet, un erou cu 30 de viata si atac zero, o mana disponibila dintr-un maxim de una.
- La fiecare meci, inclusiv restart, primul jucator este ales aleatoriu, cu sanse egale. Un spinner central se roteste 3.2 secunde, incetineste si afiseaza rezultatul timp de o secunda. Input-ul si AI-ul asteapta incheierea spinnerului; timerul ramane la 30 de secunde.
- Primul jucator pastreaza cele cinci carti initiale in prima tura. Fiecare tura urmatoare, inclusiv prima a celui de-al doilea jucator, extrage automat o carte.
- Maximul de mana creste la inceputul fiecarei ture proprii dupa prima, pana la zece. Mana se reumple, iar creaturile deja prezente devin pregatite de atac.
- Creaturile nou jucate asteapta pana la urmatoarea tura proprie, cu exceptia celor cu Charge. Fiecare creatura are un singur atac pe tura.
- Creaturile isi provoaca damage simultan si sunt eliminate la zero viata. Eroul cu atac zero nu provoaca damage inapoi.
- La moartea eroului, meciul se opreste si afiseaza rezultatul.
- Pachetul gol nu produce damage suplimentar. Nu exista o limita adaugata pentru numarul cartilor din mana sau de pe tabla; spatierea randului se adapteaza.
- Efectele sunt afisate pe carte: cost 2 = Charge, cost 3 = Tank, cost 4 = Lifesteal, cost 5 = Silence, cost 6 = Tank + Lifesteal. Costul 1 nu are efect special. Sunt cate cinci exemplare din fiecare model in fiecare pachet.
- Charge permite atacul imediat dupa plasare, dar pastreaza limita de un atac pe tura.
- Lifesteal adauga intregul ATK la HP-ul eroului dupa un atac valid, inclusiv overkill sau moartea atacatorului. Viata poate depasi 30; contraatacul aparatorului nu activeaza efectul.
- Tank obliga atacarea oricarui tank advers inaintea celorlalte carti sau a eroului. Regula se aplica si AI-ului.
- Silence cere click pe o carte adversa dupa plasare; o sageata albastra urmareste cursorul si tintele au contur albastru. Alegerea elimina toate efectele, fara a modifica ATK sau HP. O carte cu Charge, plasata recent si apoi redusa la tacere, asteapta urmatoarea tura. Fara carti adverse nu este necesara alegerea.
- Pana la alegerea tintei Silence nu pot fi jucate alte carti si nu se poate ataca. Daca expira timerul, tinta este aleasa automat, preferand Tank, apoi Lifesteal si Charge. AI-ul foloseste aceeasi prioritate.
- Nu sunt implementate spell-uri. Shield-ul demonstrativ al scheletului a fost inlocuit de atac.
- Calculatorul joaca prima carte accesibila, apoi foloseste atacurile disponibile: prefera o victorie imediata, altfel o creatura eliminabila, altfel eroul.

## Organizarea codului

Fiecare clasa are un header cu interfata si fisiere `.cpp` pentru implementare.

Componentele sunt grupate dupa responsabilitate:

- `model/`: carti, catalog, eroi, jucatori si regulile meciului.
- `ai/`: alegerea actiunilor calculatorului.
- `presentation/`: asezare, interactiuni si animatii.
- `rendering/`: randare, mesh-uri si stil vizual.
- `config/`: constante pentru reguli si animatii.
- `tema1.*`, `tema1_input.cpp`, `tema1_combat.cpp`: scena si integrarea cu framework-ul.

Include-urile folosesc cai complete pornind din `src/`. CMake descopera recursiv
fisierele din subfoldere; dupa mutari, regenerati configuratia inainte de build.
Laboratorul 4 ramane separat in `lab_m1/lab4/`; scena pornita este aleasa in `src/main.cpp`.

| Clasa | Responsabilitate | Fisiere |
| --- | --- | --- |
| `Card`, `Hero`, `Player` | Datele si operatiile creaturilor, eroilor si jucatorilor, fara OpenGL. | `model/card.*`, `model/hero.*`, `model/player.*` |
| `CardCatalog` | Definitii de carti, exemplare, etichete si prioritati Silence. | `model/card_catalog.*` |
| `CardGame` | Ture, extragere, validarea actiunilor, lupta si victorie. | `model/card_game.*` |
| `ComputerPlayer` | Alege actiunea calculatorului si prioritatea tintelor Silence; nu modifica meciul. | `ai/computer_player.*` |
| `GameLayout` | Pozitii, viewport, conversia mouse-ului si zone de selectie. Constantele comune sunt in `game_view`. | `presentation/game_layout.*` |
| `RowAnimation` | Interpoleaza pozitiile unui rand de carti. | `presentation/row_animation.*` |
| `GamePresentation` | Selectii, hover, drag, spinner, mesaje si starea animatiilor. | `presentation/game_presentation.*` |
| `GameMeshFactory` | Construieste geometria reutilizata pentru tabla, carti si simboluri. | `rendering/game_mesh_factory.*` |
| `GameRenderer` | Deseneaza meciul folosind starea vizuala doar pentru citire; detine mesh-urile si fontul. | `rendering/game_renderer.*`, `rendering/game_renderer_cards.cpp`, `rendering/game_renderer_effects.cpp` |
| `Tema1` | Conecteaza evenimentele framework-ului cu regulile si prezentarea; pastreaza ordinea actualizarilor si momentul damage-ului. | `tema1.*`, `tema1_input.cpp`, `tema1_combat.cpp` |

Pentru schimbari de reguli, punctul de plecare este `CardGame`. Pentru input, vezi
`tema1_input.cpp`; pentru sincronizarea luptei cu animatia, `tema1_combat.cpp`.
Randarea nu modifica regulile sau starea vizuala.

Animatiile de extragere, regrupare si revenire folosesc interpolare liniara bazata pe timpul dintre cadre.
Hover-ul si selectia folosesc interpolare exponentiala pentru scalare, inclusiv textul si simbolurile; hit-test-ul aplica aceeasi transformare.
Atacul dureaza aproximativ 0.90 secunde: pregatire, deplasare spre tinta, scantei la contact si revenire. Dupa contact, ambele carti se scutura prin trei oscilatii de aproximativ +/-6 grade; se rotesc inclusiv simbolurile si textul. Cartile distruse se scutura si se micsoreaza inainte de disparitie.
Damage-ul simultan se aplica o singura data la contact (0.30 secunde); eliminarea si regruparea au loc atunci. Animatia se aplica ambilor jucatori, inclusiv atacurilor asupra eroilor.
La expirarea timerului, un atac deja confirmat isi aplica damage-ul o singura data inainte de schimbarea turei.
Input-ul este blocat in timpul deplasarilor si al turei calculatorului pentru a pastra pozitiile sincronizate cu datele; scalarea nu blocheaza alegerea cartilor.

## Configurare si evitarea valorilor duplicate

- `config/game_rules.h`: viata initiala, mana, mana maxima, marimea mainii initiale, durata turei si identitatea jucatorilor.
- `model/card_catalog.cpp`: compozitia pachetului. Algoritmul genereaza pachetul din numarul de exemplare al fiecarui model; nu presupune sase modele sau 30 de carti. Efectele si prioritatile Silence sunt date explicite, independente de cost.
- `presentation/game_layout.h`: dimensiuni, margini si ancore. Eroii, mainile, pachetele si panourile se pozitioneaza relativ la scena si la zonele comune de joc.
- `config/animation_config.h`: durate, viteze si amplitudini folosite impreuna de controller si renderer.
- `rendering/game_style.*`: aspect, straturi grafice si texte. `Artwork` descrie un desen local pe care rendererul il aseaza intr-un dreptunghi destinatie. Badge-urile au aceeasi definitie pentru desenare si selectie.
- `rendering/mesh_data.*`: contururile locale ale simbolurilor si rezolutia geometriei procedurale.

Valorile care descriu echilibrarea, culorile sau forma artistica raman date de
configurare explicite. Calculele de asezare, construirea pachetului si randarea
folosesc aceste date prin rutine comune. Matricile de transformare, interpolarea
si formulele geometrice pastreaza constantele matematice necesare.

## Verificare

Din radacina proiectului:

```sh
cmake -S . -B build
cmake --build build -j 2
bash src/tests/run_tema1_tests.sh
```

Testele regulilor verifica alegerea aleatorie a primului jucator si alternarea corecta pentru ambii, amestecarea independenta, pastrarea compozitiei pachetelor, distributia initiala, mana, alternarea turelor, extragerea unica,
asteptarea dupa plasare, atacul unic, damage-ul simultan, eliminarea creaturilor,
victoria ambilor jucatori, blocarea dupa final, restartul, pachetele goale, Charge, Tank, Lifesteal si alegerea/efectele Silence.

Testele prezentarii ruleaza fara fereastra OpenGL si verifica durata spinnerului,
blocarea input-ului, viewport-ul, selectia cartilor suprapuse si scalate, revenirea
cartilor, terminarea animatiilor si prioritatile calculatorului. GLFW este necesar
doar la linkare pentru functiile de conversie a coordonatelor.

Testele configurarii verifica un pachet diferit de cel implicit (inclusiv numere
inegale de exemplare), asezarea desenelor in alte dreptunghiuri si ancorele
badge-urilor la dimensiuni diferite ale cartilor.
