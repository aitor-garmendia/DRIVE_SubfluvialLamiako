# 05 - Error Policy v1

## Regla general
Ninguna llamada BP pública debe provocar crash por mal uso.
En su lugar:
- Devuelve `false` o “result enum”
- Emite log con categoría DRIVE correspondiente
- Dispara `OnError` si es un fallo funcional relevante

## Casos mínimos cubiertos
- Start() cuando State != Ready -> false + log (Warning) + no cambia estado
- LoadScenario() cuando Running -> false + log (Warning)
- Stop() cuando ya está Ready/Uninitialized -> true (idempotente) + log Verbose opcional

## Severidad de logs
- Misuse por BP -> Warning
- Fallo interno inesperado -> Error
- Info de ciclo de vida -> Log/Verbose

## Mensajes estables
Los mensajes de error principales deben ser consistentes para troubleshooting (no variar cada build).