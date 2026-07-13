extends CharacterBody3D

@onready var agent: FlowAIAgent3D = $FlowAIAgent3D
@onready var npc_name: Label3D = $npc_name

@export var active:bool = true

func _ready() -> void:
	npc_name.text = name
	if active:
		agent.set_random_path(true)

func _physics_process(delta: float) -> void:
	if active:
		if agent.is_path_complete() or Input.is_action_just_pressed("ui_accept"):
			print("\n\n\n")
			print("NPC NAME: ", name)
			agent.set_random_path(false)
		
		var target = agent.get_next_pathnode_position()
		var direction:Vector3 = (target - global_position).normalized()
		velocity = direction * 10.0
		move_and_slide()
