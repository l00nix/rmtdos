; // void x86_dos_idle();
.global _x86_dos_idle
_x86_dos_idle:
  int    $28
  ret
