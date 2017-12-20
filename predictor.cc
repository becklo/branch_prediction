#include "predictor.h"

#define bimodal false
#define global_simple false
#define  gshare true
#define correle false
#define local false
#define mixte false
/////////////// STORAGE BUDGET JUSTIFICATION ////////////////

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// Constructeur du prédicteur
PREDICTOR::PREDICTOR(char *prog, int argc, char *argv[])
{
   // La trace est tjs présente, et les arguments sont ceux que l'on désire
   if (argc != 2) {
      fprintf(stderr, "usage: %s <trace> pcbits countbits\n", prog);
      exit(-1);
   }

   uint32_t pcbits    = strtoul(argv[0], NULL, 0);
   uint32_t countbits = strtoul(argv[1], NULL, 0);

   #if bimodal
     nentries = (1 << pcbits);        // nombre d'entrées dans la table
     pcmask   = (nentries - 1);       // masque pour n'accéder qu'aux bits significatifs de PC
     countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
     table    = new uint32_t[nentries]();
   #endif

   /////////////////////////////////////////////////////////////

   #if global_simple
     nentries = (1 << pcbits);        // nombre d'entrées dans la table
     pcmask   = (nentries - 1);       // masque pour n'accéder qu'aux bits significatifs de PC
     historicmask = (nentries - 1);   // masque pour n'accéder qu'aux bits significatifs de historic
     countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
     table    = new uint32_t[nentries]();
   #endif

   /////////////////////////////////////////////////////////////

   #if gshare
     nentries = (1 << pcbits);        // nombre d'entrées dans la table
     pcmask   = (nentries - 1);       // masque pour n'accéder qu'aux bits significatifs de PC
     historicmask = (nentries - 1);   // masque pour n'accéder qu'aux bits significatifs de historic
     countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
     table    = new uint32_t[nentries]();
   #endif
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


  bool PREDICTOR::GetPrediction(UINT64 PC)
  {
     #if bimodal
       uint32_t v = table[PC & pcmask];
       return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
     #endif

/////////////////////////////////////////////////////////////

     #if global_simple
       uint32_t v = table[historic & historicmask];
       return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
     #endif

/////////////////////////////////////////////////////////////

     #if gshare
       uint32_t v = table[(historic & historicmask)^(PC & pcmask)];
       return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
     #endif
  }

  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////

  void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
  {
     #if bimodal
       uint32_t v = table[PC & pcmask];
       table[PC & pcmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
     #endif

     /////////////////////////////////////////////////////////////

     #if global_simple
       uint32_t v = table[historic & historicmask];
       table[historic & historicmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
       historic = historic << 1 | resolveDir;
     #endif

     /////////////////////////////////////////////////////////////
     #if gshare
       uint32_t v = table[(historic & historicmask)^(PC & pcmask)];
       table[(historic & historicmask)^(PC & pcmask)] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
       historic = historic << 1 | resolveDir;
    #endif

  }


void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
   // This function is called for instructions which are not
   // conditional branches, just in case someone decides to design
   // a predictor that uses information from such instructions.
   // We expect most contestants to leave this function untouched.
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/***********************************************************/
