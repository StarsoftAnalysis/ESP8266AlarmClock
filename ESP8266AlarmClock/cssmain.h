/*! CSS Used from: http://192.168.1.50/bootstrapcss */
*,::after,::before{box-sizing:border-box;}
body{margin:0;font-family:var(--bs-body-font-family);font-size:var(--bs-body-font-size);font-weight:var(--bs-body-font-weight);line-height:var(--bs-body-line-height);color:var(--bs-body-color);text-align:var(--bs-body-text-align);background-color:var(--bs-body-bg);-webkit-text-size-adjust:100%;-webkit-tap-highlight-color:transparent;}
.h2,h1,h2,h4{margin-top:0;margin-bottom:.5rem;font-weight:500;line-height:1.2;}
h1{font-size:calc(1.375rem + 1.5vw);}
@media (min-width:1200px){
h1{font-size:2.5rem;}
}
.h2,h2{font-size:calc(1.325rem + .9vw);}
@media (min-width:1200px){
.h2,h2{font-size:2rem;}
}
h4{font-size:calc(1.275rem + .3vw);}
@media (min-width:1200px){
h4{font-size:1.5rem;}
}
p{margin-top:0;margin-bottom:1rem;}
a{color:#0d6efd;text-decoration:underline;}
a:hover{color:#0a58ca;}
svg{vertical-align:middle;}
table{caption-side:bottom;border-collapse:collapse;}
th{text-align:inherit;text-align:-webkit-match-parent;}
tbody,td,th,tr{border-color:inherit;border-style:solid;border-width:0;}
label{display:inline-block;}
button{border-radius:0;}
button:focus:not(:focus-visible){outline:0;}
button,input{margin:0;font-family:inherit;font-size:inherit;line-height:inherit;}
button{text-transform:none;}
[type=button],button{-webkit-appearance:button;}
[type=button]:not(:disabled),button:not(:disabled){cursor:pointer;}
.container{width:100%;padding-right:var(--bs-gutter-x,.75rem);padding-left:var(--bs-gutter-x,.75rem);margin-right:auto;margin-left:auto;}
@media (min-width:576px){
.container{max-width:540px;}
}
@media (min-width:768px){
.container{max-width:720px;}
}
@media (min-width:992px){
.container{max-width:960px;}
}
@media (min-width:1200px){
.container{max-width:1140px;}
}
@media (min-width:1400px){
.container{max-width:1320px;}
}
.row{--bs-gutter-x:1.5rem;--bs-gutter-y:0;display:flex;flex-wrap:wrap;margin-top:calc(var(--bs-gutter-y) * -1);margin-right:calc(var(--bs-gutter-x) * -.5);margin-left:calc(var(--bs-gutter-x) * -.5);}
.row>*{flex-shrink:0;width:100%;max-width:100%;padding-right:calc(var(--bs-gutter-x) * .5);padding-left:calc(var(--bs-gutter-x) * .5);margin-top:var(--bs-gutter-y);}
.col-auto{flex:0 0 auto;width:auto;}
@media (min-width:576px){
.col-sm{flex:1 0 0%;}
}
.table-sm>:not(caption)>*>*{padding:.25rem .25rem;}
.form-label{margin-bottom:.5rem;}
.form-control{display:block;width:100%;padding:.375rem .75rem;font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#fff;background-clip:padding-box;border:1px solid #ced4da;-webkit-appearance:none;-moz-appearance:none;appearance:none;border-radius:.25rem;transition:border-color .15s ease-in-out,box-shadow .15s ease-in-out;}
@media (prefers-reduced-motion:reduce){
.form-control{transition:none;}
}
.form-control:focus{color:#212529;background-color:#fff;border-color:#86b7fe;outline:0;box-shadow:0 0 0 .25rem rgba(13,110,253,.25);}
.form-control::placeholder{color:#6c757d;opacity:1;}
.form-control:disabled{background-color:#e9ecef;opacity:1;}
.form-check-input{width:1em;height:1em;margin-top:.25em;vertical-align:top;background-color:#fff;background-repeat:no-repeat;background-position:center;background-size:contain;border:1px solid rgba(0,0,0,.25);-webkit-appearance:none;-moz-appearance:none;appearance:none;-webkit-print-color-adjust:exact;color-adjust:exact;}
.form-check-input[type=checkbox]{border-radius:.25em;}
.form-check-input:active{filter:brightness(90%);}
.form-check-input:focus{border-color:#86b7fe;outline:0;box-shadow:0 0 0 .25rem rgba(13,110,253,.25);}
.form-check-input:checked{background-color:#0d6efd;border-color:#0d6efd;}
.form-check-input:checked[type=checkbox]{background-image:url("data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 20'%3e%3cpath fill='none' stroke='%23fff' stroke-linecap='round' stroke-linejoin='round' stroke-width='3' d='M6 10l3 3l6-6'/%3e%3c/svg%3e");}
.form-check-input:disabled{pointer-events:none;filter:none;opacity:.5;}
.btn{display:inline-block;font-weight:400;line-height:1.5;color:#212529;text-align:center;text-decoration:none;vertical-align:middle;cursor:pointer;-webkit-user-select:none;-moz-user-select:none;user-select:none;background-color:transparent;border:1px solid transparent;padding:.375rem .75rem;font-size:1rem;border-radius:.25rem;transition:color .15s ease-in-out,background-color .15s ease-in-out,border-color .15s ease-in-out,box-shadow .15s ease-in-out;}
@media (prefers-reduced-motion:reduce){
.btn{transition:none;}
}
.btn:hover{color:#212529;}
.btn:focus{outline:0;box-shadow:0 0 0 .25rem rgba(13,110,253,.25);}
.btn:disabled{pointer-events:none;opacity:.65;}
.btn-primary{color:#fff;background-color:#0d6efd;border-color:#0d6efd;}
.btn-primary:hover{color:#fff;background-color:#0b5ed7;border-color:#0a58ca;}
.btn-primary:focus{color:#fff;background-color:#0b5ed7;border-color:#0a58ca;box-shadow:0 0 0 .25rem rgba(49,132,253,.5);}
.btn-primary:active{color:#fff;background-color:#0a58ca;border-color:#0a53be;}
.btn-primary:active:focus{box-shadow:0 0 0 .25rem rgba(49,132,253,.5);}
.btn-primary:disabled{color:#fff;background-color:#0d6efd;border-color:#0d6efd;}
.navbar{position:relative;display:flex;flex-wrap:wrap;align-items:center;justify-content:space-between;padding-top:.5rem;padding-bottom:.5rem;}
.navbar>.container{display:flex;flex-wrap:inherit;align-items:center;justify-content:space-between;}
@media (min-width:768px){
.navbar-expand-md{flex-wrap:nowrap;justify-content:flex-start;}
}
.d-flex{display:flex!important;}
.h-100{height:100%!important;}
.flex-column{flex-direction:column!important;}
.mt-3{margin-top:1rem!important;}
.mt-auto{margin-top:auto!important;}
.mb-0{margin-bottom:0!important;}
.py-3{padding-top:1rem!important;padding-bottom:1rem!important;}
.text-dark{--bs-text-opacity:1;color:rgba(var(--bs-dark-rgb),var(--bs-text-opacity))!important;}
.text-white{--bs-text-opacity:1;color:rgba(var(--bs-white-rgb),var(--bs-text-opacity))!important;}
.bg-light{--bs-bg-opacity:1;background-color:rgba(var(--bs-light-rgb),var(--bs-bg-opacity))!important;}
.bg-dark{--bs-bg-opacity:1;background-color:rgba(var(--bs-dark-rgb),var(--bs-bg-opacity))!important;}

