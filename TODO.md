Vou ler o PROMPT.md para listar com precisão o que já foi entregue e o que ainda falta, e então te devolvo um checklist bem objetivo do “já dá pra fazer agora” vs “pendente”.

Read [](file:///home/cadete/Projects/Personal/raycast-game-engine/PROMPT.md#1-1), lines 1 to 260

Read [](file:///home/cadete/Projects/Personal/raycast-game-engine/PROMPT.md#260-260), lines 260 to 269

Vou checar rapidamente como está o `Entity` (se já tem parent/children ou só store flat) pra não prometer árvore de cena se ainda não existe, e aí já te mando o resumo completo.

Read [](file:///home/cadete/Projects/Personal/raycast-game-engine/engine/scene)

Read [](file:///home/cadete/Projects/Personal/raycast-game-engine/include/engine/scene/entity.h)

Searched for regex `engine\.audio|audio|engine\.ui|ui\.|engine\.world|engine\.player` (`raycast-game-engine` · `engine/scripting/lua_engine.c`), 7 results

Read [](file:///home/cadete/Projects/Personal/raycast-game-engine/engine/scripting/lua_engine.c#150-150), lines 150 to 360

Read [](file:///home/cadete/Projects/Personal/raycast-game-engine/engine)

Read [](file:///home/cadete/Projects/Personal/raycast-game-engine/engine/core)

Read [](file:///home/cadete/Projects/Personal/raycast-game-engine/include/engine)

Hoje o projeto já está bem além do “mínimo” do PROMPT.md: dá pra abrir um projeto JSON no editor, editar mapa/entidades/scripts com preview em tempo real e salvar de volta em JSON para rodar no runtime.

**O que já foi implementado (e já dá pra fazer agora)**
- Engine data-driven em JSON (sem .cub): load/save de `textures`, `colors`, `map`, `prefabs_dir`, `player_prefab`, `scene` e `player_overrides` em engine.c usando o parser/writer em json.c e json.h.
- Sistema de entidades com ID, type, transform, parent_id e properties dinâmicas (número/bool/string): entity.h, armazenamento O(1) por ID.
- Sistema de prefabs em JSON + instantiate em C: prefab.c (usado para criar o player e instâncias).
- Scripting Lua 5.4 com lifecycle `init/ready/update/destroy`: lua_engine.c + tick em engine.c.
- “Export/Inspector” funcional:
  - Defaults vêm do `export = { ... }` do script e viram properties da entidade.
  - Overrides por instância são editáveis no Inspector (properties) e agora sincronizam no preview (export ← properties) automaticamente.
- Movimento do player via Lua com colisão contra paredes, e `move_local(forward, strafe)` para WASD relativo à rotação: lua_engine.c + exemplo em player.lua.
- Editor estilo “Godot-lite” com docking + painéis:
  - Hierarchy/Inspector/Console/Content Drawer/File tree/Scene View (2D) + Viewport 3D (render-to-texture): editor_main.cpp.
- Real-time preview dentro do ImGui (framebuffer MLX → textura OpenGL → ImGui Image): editor_main.cpp.
- Botões do editor: Run/Stop, Save, Reload scripts (hot reload recriando VM Lua), Build/Export para dist: editor_main.cpp + hot reload em engine.c.
- Edição de map 2D com proteção do spawn + persistência do spawn no save (não “some” mais): editor_main.cpp + serialização em engine.c.
- Cores de chão/teto com `ColorEdit` no editor e save no project JSON: editor_main.cpp + save em engine.c.
- Persistência das velocidades do player:
  - `speed/rot_speed` agora salvam em `player_overrides` no project JSON e são aplicadas no load, então cub3D bate com o editor: engine.c.
  - Se o projeto não tinha `"scene"`, o editor agora cria automaticamente `scenes/<nome>.scene.json` ao salvar para persistir entidades não-player: editor_main.cpp.

**O que ainda falta segundo o PROMPT.md (ou está só “stub/parcial”)**
- Entity system “scene tree” completo: hoje existe `parent_id`, mas não há árvore/children/root node nem transform hierárquico de verdade (é mais um store flat): entity.h.
- API Lua mais ampla e genérica:
  - Existe `engine.entity.move/rotate/move_local`, `engine.world.is_wall`, `engine.player.id`, `engine.input.is_key_down`, `engine.ui.draw_text`.
  - Falta expor `instantiate(prefab, x, y)` para Lua e mais utilitários de entidades/scene (criar/destroi/listar, parenting, etc.): lua_engine.c.
- Physics/collision como “sistema” genérico: colisão hoje foi focada no player no binding; não existe um sistema de física geral para qualquer entidade.
- Audio system real: `engine.audio.play()` existe mas não toca nada (stub), e não há carregamento de áudio via config: lua_engine.c.
- HUD/UI scripted completo: só `ui.draw_text()`; falta `ui.draw_sprite()` e pipeline de HUD mais robusto.
- Resource management mais “engine”: além de texturas do cub3D, não há um gerenciador genérico de recursos (assets, áudio, etc).
- Requisito “JSON via cJSON”: hoje é um parser/writer interno (funcional), não cJSON como o texto pede: json.c.
- Testes: existe suite Criterion em tests, mas neste ambiente o link falha porque o lib não tem `libcriterion` compilado. Então ainda falta “test pipeline funcionando” (do jeito que o Makefile espera): Makefile.

Se você quiser, eu posso pegar o próximo item mais valioso do PROMPT.md e fechar “de verdade”: ou (A) scene tree (parent/children + transform hierárquico) ou (B) expor `engine.entity.instantiate()` no Lua + salvar instâncias via scene (workflow de gameplay bem mais completo).

## Próximo passo recomendado

Se você quiser o próximo passo do PROMPT.md, eu recomendaria: adicionar `engine.entity.instantiate(prefab, x, y)` no Lua + um botão “Add Entity” no editor (isso fecha o loop de criação sem editar JSON na mão).
