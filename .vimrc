" Configuration file for vim

set history=50				" Keep 50 lines of command line history
set ruler					" Show the cursor position all the time
set showmatch				" Show matching brackets
set wrapmargin=30			" Specify size of the right margin
set autowrite				" Automatically save buffer before commands like :next and :make
set shiftwidth=4			" Shift lines same number as tab
set tabstop=4				" Set the tab stop for four spaces
"set background=dark		" Use colors for dark background
set background=light		" Use colors for dark background
set showcmd					" Show (partial) command in status line.
set ignorecase				" Do case insensitive matching
set incsearch				" Incremental search
"set spell



set cinwords=if,else,while,do,for,switch,case
set cindent					" C smart indentations
set autoindent				" Auto indent for source code
set number					" Show line numbers
syntax on

map K {gq}
map <F2> :w<CR>:!ispell %<CR>:e %<CR>


