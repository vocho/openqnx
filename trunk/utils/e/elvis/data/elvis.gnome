gui defaction /Open/ /:e$open$/ /open/ // /open.cancel/ /open.ok/
gui defaction /SaveAs/ /:w $saveas$/ /saveas/ // /saveas.cancel/ /saveas.ok/
gui defaction /SearchReplace/ @:.,$$s/$searchreplace.searchstring$/$searchreplace.replacestring$/$searchreplace.global?(g):()$searchreplace.confirm?(c):()@ /searchreplace/ /searchreplace.apply/ /searchreplace.cancel/ /searchreplace.ok/
gui defaction /about/ /:version/ /about/ // // //

gui menubar addentry /File/
gui menu addentry /File/ /New/ /:new/
gui menu addentry /File/ /Open/ ":gui open /Open/"
gui menu addentry /File/ /Save/ /:w/
gui menu addentry /File/ /Save as/ ":gui open /SaveAs/"
gui menu addentry /File/ /Close/ /:q/

gui menubar addentry /Edit/
gui menu addentry /Edit/ /Search & Replace/ ":gui open /SearchReplace/"

gui menubar addentry /View/
gui menu addentry /View/ /Normal/ /:no/
gui menu addentry /View/ /Hex/ /:display hex/


gui menubar addentry /Help/
gui menu addentry /Help/ /Manual/ /:help/
gui menu addentry /Help/ /About/ ":gui open /about/"

gui toolbar addentry "New" "New" "Create a new window for a new file" ":new"
gui toolbar addentry "Open" "Open" "Open an existing file" ":gui open /Open/"
gui toolbar addentry "Save" "Save" "Save the buffer" ":w"
