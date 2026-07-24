# DRIVE - Blueprint Public API v1
## Scope & Principles

### Alcance de v1
Esta especificación define la API pública oficial de DRIVE para usuarios Blueprint.
Incluye: piezas disponibles (actores/componentes/subsystem/librería), propiedades expuestas, eventos soportados y flujo de uso garantizado.

No incluye implementación C++ ni Blueprints finales.

### Principios
1. **Una sola puerta de entrada BP**: los usuarios BP no tocan Core directamente.
2. **API estable**: nombres, categorías y responsabilidades se congelan en v1.
3. **Estados seguros**: llamadas fuera de orden no crashean; devuelven error/false y log.
4. **Flujo reproducible**: LoadScenario → Start → Running → Stop.
5. **Sin responsabilidades mezcladas**: el “qué” (simulación) vive en Core; BP solo orquesta/observa.